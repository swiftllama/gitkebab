
#include <string.h>

#include "git2.h"
#include "gk_index.h"
#include "gk_results.h"
#include "gk_logging.h"

static git_index *index_for_session(gk_session *session, const char* purpose, const char *path) {
    gk_result *result = NULL;
    git_index *index = NULL;
    int rc;
    
    if (session == NULL) {
        log_error(COMP_COMMIT, "Cannot %s, session is NULL", purpose);
        return NULL;
    }

    if (session->state.local_checkout_exists == 0) {
        gk_session_failure(session, &COMP_COMMIT, -3, "Cannot %s, local checkout does not exist", purpose);
        return NULL;
    }
    else if (session->lg2_repository == NULL) {
        // should never happen if local_checkout_exists == 1
        gk_session_failure(session, &COMP_COMMIT, -3, "Cannot %s, internal git2 repository is unexpectedely NULL", purpose);
        return NULL;
    }
    else if (path == NULL) {
        gk_session_failure(session, &COMP_COMMIT, -3, "Cannot %s, path/pattern is NULL", purpose);
        return NULL;
    }
    else if (path == "") {
        gk_session_failure(session, &COMP_COMMIT, -3, "Cannot %s, path/pattern is empty", purpose);
        return NULL;
    }

    rc = git_repository_index(&index, session->lg2_repository);
    if (rc != 0) {
        const git_error *err = git_error_last();
        gk_session_failure(session, &COMP_COMMIT, -3, "Cannot %s, error obtaining repository index (%d) while: %s", purpose, err->klass, err->message);
        git_index_free(index);
        return NULL;
    }

    return index;
}

int gk_session_index_add_path(gk_session *session, const char *path) {
    gk_result *result = NULL;

    git_index *index = index_for_session(session, "add path to index", path);
    if (index == NULL) {
        return GK_FAILURE;
    }

    int rc = git_index_add_bypath(index, path);
    git_index_free(index);
    
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Error adding path '%s' to repository index (%d): %s", path, err->klass, err->message);
    }
    
    return gk_session_success(session);
}

int gk_session_index_remove_path(gk_session *session, const char *path) {
    gk_result *result = NULL;

    git_index *index = index_for_session(session, "remove path from index", path);
    if (index == NULL) {
        return GK_FAILURE;
    }

    int rc = git_index_remove_bypath(index, path);
    git_index_free(index);

    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Error removing path '%s' from repository index (%d): %s", path, err->klass, err->message);
    }
    
    return gk_session_success(session);
}

int gk_session_index_add_all(gk_session *session, const char* pattern) {
    gk_result *result = NULL;

    git_index *index = index_for_session(session, "add all to index", pattern);
    if (index == NULL) {
        return GK_FAILURE;
    }

    char *path_pattern = strdup(pattern);
    git_strarray paths = {&path_pattern, 1};
    int rc = git_index_add_all(index, &paths, GIT_INDEX_ADD_DEFAULT, NULL, NULL);
    free(path_pattern);
    git_index_free(index);

    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Error adding all paths matching '%s' to repository index (%d): %s", pattern, err->klass, err->message);
    }

    return gk_session_success(session);
}

int gk_session_index_update_all(gk_session *session, const char* pattern) {
    gk_result *result = NULL;
    
    git_index *index = index_for_session(session, "update all in index", pattern);
    if (index == NULL) {
        return GK_FAILURE;
    }

    char *path_pattern = strdup(pattern);
    git_strarray paths = {&path_pattern, 1};
    int rc = git_index_update_all(index, &paths, NULL, NULL);
    free(path_pattern);
    git_index_free(index);
            
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Error updating all paths matching '%s' in the repository index (%d): %s", pattern, err->klass, err->message);
    }

    return gk_session_success(session);
}
