
#include "git2.h"
#include "gk_index.h"
#include "gk_results.h"
#include "gk_logging.h"

int gk_session_add_path_to_index(gk_session_t *session, const char *path) {
    gk_result_t *result = NULL;
    
    if (session == NULL) {
        log_error(COMP_COMMIT, "Cannot retrieve add path to index, session is NULL");
        return GK_FAILURE;
    }
    if (session->state.local_checkout_exists == 0) {
        result = gk_result(-1, "Cannot add to index, local checkout does not exist");
        log_error(COMP_COMMIT, gk_result_message(result));
        gk_session_set_last_result(session, result);
        return GK_FAILURE;
    }
    if (session->lg2_repository == NULL) {
        // should never happen if local_checkout_exists == 1
        result = gk_result(-1, "Cannot add to index, internal git2 repository is unexpectedely NULL");
        log_error(COMP_COMMIT, gk_result_message(result));
        gk_session_set_last_result(session, result);
        return GK_FAILURE;
    }
    if ((path == NULL) || (path == "")) {
        result = gk_result(-1, path == NULL ? "Cannot add NULL path to index" : "Cannot add empty path to index");
        log_error(COMP_COMMIT, gk_result_message(result));
        gk_session_set_last_result(session, result);
        return GK_FAILURE;
    }

    git_index *index = NULL;
    int rc = git_repository_index(&index, session->lg2_repository);

    if (rc != 0) {
        char message[256];
        const git_error *err = git_error_last();
        snprintf(message, 256, "Error obtaining repository index (%d): %s", err->klass, err->message);
        result = gk_result(-2, message);
        log_error(COMP_COMMIT, gk_result_message(result));
        gk_session_set_last_result(session, result);        
        if (index != NULL) {
            git_index_free(index);
        }
        return GK_FAILURE;
    }

    rc = git_index_add_bypath(index, path);

    if (rc != 0) {
        char message[512];
        const git_error *err = git_error_last();
        snprintf(message, 512, "Error adding path '%s' to repository index (%d): %s", path, err->klass, err->message);
        result = gk_result(-3, message);
        log_error(COMP_COMMIT, gk_result_message(result));
        gk_session_set_last_result(session, result);        
        if (index != NULL) {
            git_index_free(index);
        }
        return GK_FAILURE;
    }
    
    if (index != NULL) {
        git_index_free(index);
    }
    gk_session_set_last_result(session, gk_result_success());
    return GK_SUCCESS;
}
