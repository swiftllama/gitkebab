
#include <string.h>

#include "git2.h"
#include "gk_index.h"
#include "gk_results.h"
#include "gk_logging.h"
#include "gk_lg2_private.h"

static git_index *index_for_session(gk_session *session, const char* purpose, const char *path) {
    if (gk_session_verify(session, &COMP_COMMIT, GK_SESSION_VERIFY_LOCAL_CHECKOUT, purpose) != GK_SUCCESS) {
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

    if (gk_lg2_index_load(session, purpose) == GK_FAILURE) {
        return NULL;
    }
}

static int verify_session_and_path(gk_session *session, const char* purpose, const char *path) {
    if (gk_session_verify(session, &COMP_COMMIT, GK_SESSION_VERIFY_LOCAL_CHECKOUT, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (path == NULL) {
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot %s, path/pattern is NULL", purpose);
    }
    else if (path == "") {
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot %s, path/pattern is empty", purpose);
    }
    return GK_SUCCESS;
}

int gk_session_index_add_path(gk_session *session, const char *path) {
    if (verify_session_and_path(session, "add path to index", path) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    if (gk_lg2_index_load(session, "add path to index") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    int rc = git_index_add_bypath(session->lg2_resources->index, path);
    gk_lg2_index_free(session);
    
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Error adding path '%s' to repository index (%d): %s", path, err->klass, err->message);
    }
    
    return gk_session_success(session);
}

int gk_session_index_remove_path(gk_session *session, const char *path) {
    if (verify_session_and_path(session, "remove path from index", path) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    if (gk_lg2_index_load(session, "remove path from index") != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    int rc = git_index_remove_bypath(session->lg2_resources->index, path);
    gk_lg2_index_free(session);

    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Error removing path '%s' from repository index (%d): %s", path, err->klass, err->message);
    }
    
    return gk_session_success(session);
}

int gk_session_index_add_all(gk_session *session, const char* pattern) {
    if (verify_session_and_path(session, "add all paths", pattern) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    if (gk_lg2_index_load(session, "add all paths") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    char *path_pattern = strdup(pattern);
    git_strarray paths = {&path_pattern, 1};
    int rc = git_index_add_all(session->lg2_resources->index, &paths, GIT_INDEX_ADD_DEFAULT, NULL, NULL);
    free(path_pattern);
    gk_lg2_index_free(session);

    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Error adding all paths matching '%s' to repository index (%d): %s", pattern, err->klass, err->message);
    }

    return gk_session_success(session);
}

int gk_session_index_update_all(gk_session *session, const char* pattern) {
    if (verify_session_and_path(session, "update all paths", pattern) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    if (gk_lg2_index_load(session, "update all paths") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    char *path_pattern = strdup(pattern);
    git_strarray paths = {&path_pattern, 1};
    int rc = git_index_update_all(session->lg2_resources->index, &paths, NULL, NULL);
    free(path_pattern);
    gk_lg2_index_free(session);

    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Error updating all paths matching '%s' to repository index (%d): %s", pattern, err->klass, err->message);
    }

    return gk_session_success(session);
}
