
#include "git2.h"
#include "gk_index.h"
#include "gk_results.h"
#include "gk_logging.h"

static git_index *index_for_session(gk_session_t *session, const char* purpose, const char *path) {
    gk_result_t *result = NULL;
    
    if (session == NULL) {
        log_error(COMP_COMMIT, "Cannot %s, session is NULL", purpose);
        return NULL;
    }

    git_index *index = NULL;
    char message[256] = {0};
    if (session->state.local_checkout_exists == 0) {
        snprintf(message, 256, "Cannot %s, local checkout does not exist", purpose);
    }
    else if (session->lg2_repository == NULL) {
        snprintf(message, 256, "Cannot %s, internal git2 repository is unexpectedely NULL", purpose);
        // should never happen if local_checkout_exists == 1
    }
    else if (path == NULL) {
        snprintf(message, 256, "Cannot %s, path is NULL", purpose);
    }
    else if (path == "") {
        snprintf(message, 256, "Cannot %s, path is empty", purpose);
    }
    else {
        int rc = git_repository_index(&index, session->lg2_repository);
        if (rc != 0) {
            const git_error *err = git_error_last();
            snprintf(message, 256, "Cannot %s, error obtaining repository index (%d) while: %s", purpose, err->klass, err->message);
        }
    }

    if (message[0] != '\0') {
        result = gk_result(-1, message);
        log_error(COMP_COMMIT, gk_result_message(result));
        gk_session_set_last_result(session, result);
        if (index != NULL) {
            git_index_free(index);
        }
        return NULL;
    }

    return index;


}

int gk_session_add_path_to_index(gk_session_t *session, const char *path) {
    gk_result_t *result = NULL;
    
    git_index *index = index_for_session(session, "add path to index", path);
    if (index == NULL) {
        return GK_FAILURE;
    }

    int rc = git_index_add_bypath(index, path);

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

int gk_session_remove_path_from_index(gk_session_t *session, const char *path) {
    gk_result_t *result = NULL;
    
    git_index *index = index_for_session(session, "remove path from index", path);
    if (index == NULL) {
        return GK_FAILURE;
    }

    int rc = git_index_remove_bypath(index, path);

    if (rc != 0) {
        char message[512];
        const git_error *err = git_error_last();
        snprintf(message, 512, "Error removing path '%s' from repository index (%d): %s", path, err->klass, err->message);
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
