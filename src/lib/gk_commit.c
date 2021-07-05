
#include "gk_commit.h"
#include "gk_logging.h"
#include "git2.h"

size_t gk_session_count_reflog_entries(gk_session_t *session) {
    gk_result_t *result = NULL;
    
    if (session == NULL) {
        log_error(COMP_COMMIT, "Cannot count reflog entries, session is NULL");
        return 0;
    }
    git_index *index = NULL;
    char message[256] = {0};
    if (session->state.local_checkout_exists == 0) {
        snprintf(message, 256, "Cannot count reflog entries, local checkout does not exist");
    }
    else if (session->lg2_repository == NULL) {
        snprintf(message, 256, "Cannot count reflog entries, internal git2 repository is unexpectedely NULL");
        // should never happen if local_checkout_exists == 1
    }

    if (message[0] != '\0') {
        result = gk_result(-1, message);
        log_error(COMP_COMMIT, gk_result_message(result));
        gk_session_set_last_result(session, result);
        if (index != NULL) {
            git_index_free(index);
        }
        return 0;
    }

    git_reflog *reflog = NULL;
    int rc = git_reflog_read(&reflog, session->lg2_repository, "HEAD");
    size_t entrycount = rc == 0 ? git_reflog_entrycount(reflog) : 0;
    if (reflog != NULL) {
        git_reflog_free(reflog);
    }
    
    if (rc != 0) {
        const git_error *err = git_error_last();
        snprintf(message, 512, "Error counting reflog entries (%d): %s", err->klass, err->message);
        result = gk_result(-2, message);
        log_error(COMP_COMMIT, gk_result_message(result));
        gk_session_set_last_result(session, result);        
    }
    else {
        gk_session_set_last_result(session, gk_result_success());
    }

    return entrycount;
}
