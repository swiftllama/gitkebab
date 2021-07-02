
#include "git2.h"
#include "gk_status.h"
#include "gk_logging.h"

void gk_status_summary_reset(gk_status_summary_t *status_summary) {
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

int gk_session_query_status_summary(gk_session_t *session) {
    gk_result_t *result = NULL;

    if (session == NULL) {
        log_error(COMP_STATUS, "Cannot check status, session is NULL");
        return GK_FAILURE;
    }
    if (session->state.local_checkout_exists == 0) {
        result = gk_result(-1, "Cannot query status, local checkout does not exist");
        log_error(COMP_STATUS, gk_result_message(result));
        gk_session_set_last_result(session, result);
        return GK_FAILURE;
    }
    if (session->lg2_repository == NULL) {
        // Should never happen if local_checkout_exists == 1
        result = gk_result(-1, "Cannot query status, internal git2 repository is unexpectedely NULL");
        log_error(COMP_STATUS, gk_result_message(result));
        gk_session_set_last_result(session, result);
        return GK_FAILURE;
    }
    
    if (session->lg2_status_list != NULL) {
        git_status_list_free(session->lg2_status_list);
        session->lg2_status_list = NULL;
    }
    
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
        char message[256];
        const git_error *err = git_error_last();
        snprintf(message, 256, "Error querying status (%d): %s", err->klass, err->message);
        result = gk_result(-2, message);
        log_error(COMP_STATUS, gk_result_message(result));
        gk_session_set_last_result(session, result);
        return GK_FAILURE;
    }

    session->lg2_status_list = (void *)status_list;

    size_t count_total = git_status_list_entrycount(status_list);
    gk_status_summary_reset(&session->status_summary);
    for (size_t i = 0; i < count_total; i += 1) {
        const git_status_entry *entry = git_status_byindex(status_list, i);
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

    gk_session_set_last_result(session, gk_result_success());
    return GK_SUCCESS;
}

const char *gk_session_status_summary_path_at(gk_session_t *session, size_t index) {
    gk_result_t *result = NULL;
    if (session == NULL) {
        log_error(COMP_STATUS, "Cannot retrieve path summary at index %zu, session is NULL", index);
        return "";
    }
    git_status_list *status_list = (git_status_list *)session->lg2_status_list;
    if (status_list == NULL) {
        char message[256];
        snprintf(message, 256, "Error retrieving path at index %zu, status_list is NULL", index);
        result = gk_result(-1, message);
        log_error(COMP_STATUS, gk_result_message(result));
        gk_session_set_last_result(session, result);
        return "";
    }
    size_t total = git_status_list_entrycount(status_list);
    if (index >= total) {
        char message[256];
        snprintf(message, 256, "Error retrieving path at index %zu: index out of bounds (total entry count is only %zu)", index, total);
        result = gk_result(-1, message);
        log_error(COMP_STATUS, gk_result_message(result));
        gk_session_set_last_result(session, result);
        return "";
    }

    const git_status_entry *entry = git_status_byindex(status_list, index);
    if (entry == NULL) {
        log_error(COMP_STATUS, "Unexpected status list NULL entry at index %zu", index);
        return "";
    }

    const char *path = "";
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
    }

    return path;
}

size_t gk_session_status_summary_entrycount(gk_session_t *session) {
    gk_result_t *result = NULL;
    if (session == NULL) {
        log_error(COMP_STATUS, "Cannot retrieve summary entry count, session is NULL");
        return 0;
    }
    git_status_list *status_list = (git_status_list *)session->lg2_status_list;
    if (status_list == NULL) {
        result = gk_result(-1, "Error retrieving summary entry count, status list is NULL");
        log_error(COMP_STATUS, gk_result_message(result));
        gk_session_set_last_result(session, result);
        return 0;
    }
    return git_status_list_entrycount(status_list);
}
