
#include "git2.h"
#include "gk_status.h"
#include "gk_logging.h"
#include "gk_lg2_private.h"

void gk_status_summary_reset(gk_status_summary *status_summary) {
    if (status_summary == NULL) {
        log_error(COMP_STATUS, "Cannot reset NULL status summary");
        return;
    }

    status_summary->count_new = 0;
    status_summary->count_modified = 0;
    status_summary->count_deleted = 0;
    status_summary->count_renamed = 0;
    status_summary->count_typechange = 0;
    status_summary->count_conflicted = 0;
}

void gk_session_status_summary_close(gk_session *session) {
    gk_lg2_status_list_free(session);
}

int gk_session_status_summary_query(gk_session *session) {
    gk_result *result = NULL;

    if (gk_session_verify(session, &COMP_STATUS, GK_SESSION_VERIFY_LOCAL_CHECKOUT, "query status") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_lg2_status_list_free(session);
    if (gk_lg2_status_list_load(session, "query status") != GK_SUCCESS) {
        gk_lg2_status_list_free(session);
        return GK_FAILURE;
    }

    size_t count_total = git_status_list_entrycount(session->lg2_resources->status_list);
    gk_status_summary_reset(&session->status_summary);
    for (size_t i = 0; i < count_total; i += 1) {
        const git_status_entry *entry = git_status_byindex(session->lg2_resources->status_list, i); // NOTE: entry should not be freed
        if (entry == NULL) {
            log_error(COMP_STATUS, "Unexpected status list NULL entry at index %d", i);
            continue;
        }
        if (((entry->status & GIT_STATUS_INDEX_NEW) ==  GIT_STATUS_INDEX_NEW) ||
            ((entry->status & GIT_STATUS_WT_NEW) ==  GIT_STATUS_WT_NEW)) {
            session->status_summary.count_new += 1;
        }
        else if (((entry->status & GIT_STATUS_INDEX_MODIFIED) ==  GIT_STATUS_INDEX_MODIFIED) ||
                 ((entry->status & GIT_STATUS_WT_MODIFIED) ==  GIT_STATUS_WT_MODIFIED)) {
            session->status_summary.count_modified += 1;
        }
        else if (((entry->status & GIT_STATUS_INDEX_DELETED) ==  GIT_STATUS_INDEX_DELETED) ||
                 ((entry->status & GIT_STATUS_WT_DELETED) ==  GIT_STATUS_WT_DELETED)) {
            session->status_summary.count_deleted += 1;
        }
        else if (((entry->status & GIT_STATUS_INDEX_RENAMED) ==  GIT_STATUS_INDEX_RENAMED) ||
                 ((entry->status & GIT_STATUS_WT_RENAMED) ==  GIT_STATUS_WT_RENAMED)) {
            session->status_summary.count_renamed += 1;
        }
        else if (((entry->status & GIT_STATUS_INDEX_TYPECHANGE) ==  GIT_STATUS_INDEX_TYPECHANGE) ||
                 ((entry->status & GIT_STATUS_WT_TYPECHANGE) ==  GIT_STATUS_WT_TYPECHANGE)) {
            session->status_summary.count_typechange += 1;
        }
        else if ((entry->status & GIT_STATUS_CONFLICTED) == GIT_STATUS_CONFLICTED) {
            session->status_summary.count_conflicted += 1;
        }
    }

    if (session->status_summary.count_conflicted > 0) {
        gk_session_state_set(session, GK_SESSION_STATE_HAS_CONFLICTS);
    }
    else {
        gk_session_state_unset(session, GK_SESSION_STATE_HAS_CONFLICTS);
    }
        
    int status = git_repository_state(session->lg2_resources->repository);
    if (status == GIT_REPOSITORY_STATE_MERGE) {
        gk_session_state_set(session, GK_SESSION_STATE_MERGE_FINALIZATION_PENDING);
    }
    else {
        gk_session_state_unset(session, GK_SESSION_STATE_MERGE_FINALIZATION_PENDING);
    }

    return gk_session_success(session);
}

static const git_status_entry *gk_session_status_summary_entry(gk_session *session, const char *purpose, size_t index) {
    if (gk_session_verify(session, &COMP_STATUS, GK_SESSION_VERIFY_LOCAL_CHECKOUT | GK_SESSION_VERIFY_STATUS_LIST, "retrieve status list entry") != GK_SUCCESS) {
        return NULL;
    }

    size_t total = git_status_list_entrycount(session->lg2_resources->status_list);
    if (index >= total) {
        gk_session_failure(session, &COMP_STATUS, -1, "Error retrieving %s at index %zu, index out of bounds (total entry count is only %zu)", purpose, index, total);
        return NULL;
    }
    const git_status_entry *entry = git_status_byindex(session->lg2_resources->status_list, index);    
    if (entry == NULL) {
        gk_session_failure(session, &COMP_STATUS, -1, "Error retrieving %s at index %zu, status list entry is unexpectedly NULL", purpose, index, total);
    }
    return entry;
}

int gk_session_status_summary_status_at(gk_session *session, size_t index) {
    const git_status_entry *entry = gk_session_status_summary_entry(session, "status", index);
    return entry == NULL ? 0 : entry->status;
}

const char *gk_session_status_summary_path_at(gk_session *session, size_t index) {
    const git_status_entry *entry = gk_session_status_summary_entry(session, "status", index);
    if (entry == NULL) {
        return "";
    }

    const char *path = NULL;
    if (entry->index_to_workdir != NULL) {
        if (entry->index_to_workdir->old_file.path != NULL) {
            path = entry->index_to_workdir->old_file.path;
        }
        if ((path == NULL) && (entry->index_to_workdir->new_file.path != NULL)) {
            path = entry->index_to_workdir->new_file.path;
        }
    }
    if ((path == NULL) && (entry->head_to_index != NULL)) {
        if ((path == NULL) && (entry->head_to_index->old_file.path != NULL)) {
            path = entry->head_to_index->old_file.path;
        }
        if ((path == NULL) && (entry->head_to_index->new_file.path != NULL)) {
            path = entry->head_to_index->new_file.path;
        }
    }

    if (path == NULL) {
        log_warn(COMP_STATUS, "error finding path for status entry at index %d, all values are NULL", index);
        return "";
    }

    return path;
}

size_t gk_session_status_summary_entrycount(gk_session *session) {
    if (gk_session_verify(session, &COMP_STATUS, GK_SESSION_VERIFY_LOCAL_CHECKOUT | GK_SESSION_VERIFY_STATUS_LIST, "retrieve status list entry count") != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    return git_status_list_entrycount(session->lg2_resources->status_list);
}
