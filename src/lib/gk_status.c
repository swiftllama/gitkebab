
#include "git2.h"
#include "gk_status.h"
#include "gk_logging.h"

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

int gk_session_query_status_summary(gk_session *session) {
    gk_result *result = NULL;

    if (session == NULL) {
        log_error(COMP_STATUS, "Cannot check status, session is NULL");
        return GK_FAILURE;
    }
    if (gk_session_state_disabled(session, GK_SESSION_STATE_LOCAL_CHECKOUT_EXISTS)) {
        return gk_session_failure(session, &COMP_STATUS, -1, "Cannot query status, local checkout does not exist");
    }
    if (session->lg2_repository == NULL) {
        // Should never happen if local_checkout_exists == 1
        return gk_session_failure(session, &COMP_STATUS, -1, "Cannot query status, internal git2 repository is unexpectedely NULL");
    }
    
    git_status_list_free(session->lg2_status_list);
    session->lg2_status_list = NULL;
    
    git_status_options status_options = GIT_STATUS_OPTIONS_INIT;
    
    status_options.show  = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    status_options.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
        GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS |
        GIT_STATUS_OPT_RENAMES_INDEX_TO_WORKDIR | 
        GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
        GIT_STATUS_OPT_SORT_CASE_INSENSITIVELY;

    git_status_list *status_list = NULL;
    int rc = git_status_list_new(&status_list, (git_repository *)session->lg2_repository, &status_options);
    if (rc != 0) {
        git_status_list_free(status_list);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_STATUS, -2, "Error querying status (%d): %s", err->klass, err->message);
    }

    session->lg2_status_list = (void *)status_list;

    size_t count_total = git_status_list_entrycount(status_list);
    gk_status_summary_reset(&session->status_summary);
    for (size_t i = 0; i < count_total; i += 1) {
        const git_status_entry *entry = git_status_byindex(status_list, i); // NOTE: entry should not be freed
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

    return gk_session_success(session);
}

static const git_status_entry *gk_session_status_summary_entry(gk_session *session, const char *purpose, size_t index) {
    gk_result *result = NULL;
    if (session == NULL) {
        log_error(COMP_STATUS, "Cannot retrieve %s at index %zu, session is NULL", purpose, index);
        return NULL;
    }
    git_status_list *status_list = (git_status_list *)session->lg2_status_list;
    if (status_list == NULL) {
        gk_session_failure(session, &COMP_STATUS, -1, "Error retrieving %s at index %zu, status_list is NULL", purpose, index);
        return NULL;
    }
    size_t total = git_status_list_entrycount(status_list);
    if (index >= total) {
        gk_session_failure(session, &COMP_STATUS, -1, "Error retrieving %s at index %zu, index out of bounds (total entry count is only %zu)", purpose, index, total);
        return NULL;
    }
    const git_status_entry *entry = git_status_byindex(status_list, index);    
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
    gk_result *result = NULL;
    if (session == NULL) {
        log_error(COMP_STATUS, "Cannot retrieve summary entry count, session is NULL");
        return 0;
    }
    git_status_list *status_list = (git_status_list *)session->lg2_status_list;
    if (status_list == NULL) {
        gk_session_failure(session, &COMP_STATUS, -1, "Error retrieving summary entry count, status list is NULL");
        return 0;
    }
    return git_status_list_entrycount(status_list);
}
