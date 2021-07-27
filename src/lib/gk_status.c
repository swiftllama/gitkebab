
#include "git2.h"
#include "gk_status.h"
#include "gk_logging.h"
#include "gk_lg2_private.h"
#include "gk_session.h"

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

void gk_status_summary_close(gk_session *session) {
    gk_lg2_status_list_free(session->repository);
}

int gk_status_summary_query(gk_session *session) {
    const char *purpose = "query status";
    if (gk_session_context_push(session, purpose, &COMP_STATUS, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    gk_repository *repository = session->repository;

    gk_lg2_status_list_free(repository);
    if (gk_lg2_status_list_load(session) != GK_SUCCESS) {
        gk_lg2_status_list_free(repository);
        return gk_session_failure(session);
    }

    size_t count_total = git_status_list_entrycount(lg2_resources->status_list);
    gk_status_summary_reset(&session->repository->status_summary);
    for (size_t i = 0; i < count_total; i += 1) {
        const git_status_entry *entry = git_status_byindex(lg2_resources->status_list, i); // NOTE: entry should not be freed
        if (entry == NULL) {
            log_error(COMP_STATUS, "Unexpected status list NULL entry at index %d", i);
            continue;
        }
        if (((entry->status & GIT_STATUS_INDEX_NEW) ==  GIT_STATUS_INDEX_NEW) ||
            ((entry->status & GIT_STATUS_WT_NEW) ==  GIT_STATUS_WT_NEW)) {
            repository->status_summary.count_new += 1;
        }
        else if (((entry->status & GIT_STATUS_INDEX_MODIFIED) ==  GIT_STATUS_INDEX_MODIFIED) ||
                 ((entry->status & GIT_STATUS_WT_MODIFIED) ==  GIT_STATUS_WT_MODIFIED)) {
            repository->status_summary.count_modified += 1;
        }
        else if (((entry->status & GIT_STATUS_INDEX_DELETED) ==  GIT_STATUS_INDEX_DELETED) ||
                 ((entry->status & GIT_STATUS_WT_DELETED) ==  GIT_STATUS_WT_DELETED)) {
            repository->status_summary.count_deleted += 1;
        }
        else if (((entry->status & GIT_STATUS_INDEX_RENAMED) ==  GIT_STATUS_INDEX_RENAMED) ||
                 ((entry->status & GIT_STATUS_WT_RENAMED) ==  GIT_STATUS_WT_RENAMED)) {
            repository->status_summary.count_renamed += 1;
        }
        else if (((entry->status & GIT_STATUS_INDEX_TYPECHANGE) ==  GIT_STATUS_INDEX_TYPECHANGE) ||
                 ((entry->status & GIT_STATUS_WT_TYPECHANGE) ==  GIT_STATUS_WT_TYPECHANGE)) {
            repository->status_summary.count_typechange += 1;
        }
        else if ((entry->status & GIT_STATUS_CONFLICTED) == GIT_STATUS_CONFLICTED) {
            repository->status_summary.count_conflicted += 1;
        }
    }

    if (repository->status_summary.count_conflicted > 0) {
        gk_repository_state_set(repository, GK_REPOSITORY_STATE_HAS_CONFLICTS);
        log_info(COMP_STATUS, "Repository has conflicts");
    }
    else {
        gk_repository_state_unset(repository, GK_REPOSITORY_STATE_HAS_CONFLICTS);
        log_info(COMP_STATUS, "Repository is conflict free");
    }
        
    int status = git_repository_state(lg2_resources->repository);
    if (status == GIT_REPOSITORY_STATE_MERGE) {
        gk_repository_state_set(repository, GK_REPOSITORY_STATE_MERGE_PENDING_ON_DISK);
        log_info(COMP_STATUS, "Repository is has a pending merge on disk");
    }
    else {
        gk_repository_state_unset(repository, GK_REPOSITORY_STATE_MERGE_PENDING_ON_DISK);
    }

    return gk_session_success(session, purpose);
}

static const git_status_entry *gk_status_summary_entry(gk_session *session, size_t index) {
    const char *purpose = "get status summary entry";
    if (gk_session_context_push(session, purpose, &COMP_REMOTE, GK_REPOSITORY_VERIFY_DEFAULT | GK_REPOSITORY_VERIFY_STATUS_LIST) != GK_SUCCESS) {
        return NULL;
    }

    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    
    size_t total = git_status_list_entrycount(lg2_resources->status_list);
    if (index >= total) {
        gk_session_failure_ex(session, purpose, GK_ERR, "Failed to retrieve status entry at index %zu, index out of bounds (total entry count is only %zu)", index, total);
        return NULL;
    }
    const git_status_entry *entry = git_status_byindex(lg2_resources->status_list, index);    
    if (entry == NULL) {
        gk_session_failure_ex(session, purpose, GK_ERR, "Failed to retrieve status entry at index %zu, status list entry is unexpectedly NULL", index, total);
        return NULL;
    }

    gk_session_success(session, purpose);
    return entry;
}

int gk_status_summary_status_at(gk_session *session, size_t index) {
    const git_status_entry *entry = gk_status_summary_entry(session, index);
    return entry == NULL ? 0 : entry->status;
}

const char *gk_status_summary_path_at(gk_session *session, size_t index) {
    const git_status_entry *entry = gk_status_summary_entry(session, index);
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
        log_warn(COMP_STATUS, "error finding path for status entry at index [%zu], all values are NULL", index);
        return "";
    }

    return path;
}

size_t gk_status_summary_entrycount(gk_session *session) {
    const char *purpose = "fetch from remote";
    if (gk_session_context_push(session, purpose, &COMP_STATUS, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT | GK_REPOSITORY_VERIFY_STATUS_LIST) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    size_t entrycount = git_status_list_entrycount(session->repository->lg2_resources->status_list);
    gk_session_success(session, purpose);
    return entrycount;
}
