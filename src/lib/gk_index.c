
#include <string.h>

#include "git2.h"
#include "gk_index.h"
#include "gk_results.h"
#include "gk_logging.h"
#include "gk_lg2_private.h"

int gk_index_add_path(gk_session *session, const char *path) {
    const char *purpose = "add path to index";
    if (gk_session_context_push(session, purpose, &COMP_COMMIT, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (path == NULL) {
        return gk_sesion_failure_ex(session, purpose, GK_ERR, "path is NULL");
    }
    
    if (gk_lg2_index_load(session) != GK_SUCCESS) {
        gk_lg2_index_free(session);
        return gk_session_failure(session, purpose, GK_ERR);
    }

    if (git_index_add_bypath(repository->lg2_resources->index, path) != 0) {
        gk_lg2_index_free(session);
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to add [%s] to index", path);
    }

    gk_lg2_index_free(repository);
    return gk_session_success(session, purpose);
}

int gk_repository_index_remove_path(gk_session *session, const char *path) {
    const char *purpose = "remove path from index";
    if (gk_session_context_push(session, purpose, &COMP_CONFLICTS, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (path == NULL) {
        return gk_sesion_failure_ex(session, purpose, GK_ERR, "path is NULL");
    }
    
    if (gk_lg2_index_load(session) != GK_SUCCESS) {
        gk_lg2_index_free(session);
        return gk_session_failure(session, purpose, GK_ERR);
    }
    
    if (git_index_remove_bypath(repository->lg2_resources->index, path) != 0) {
        gk_lg2_index_free(session);
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to remove [%s] from index", path);        
    }
    gk_lg2_index_free(session);
    
    return gk_session_success(session, purpose);
}

int gk_repository_index_add_all(gk_session *session, const char* pattern) {
    const char *purpose = "add all files to index";
    if (gk_session_context_push(session, purpose, &COMP_CONFLICTS, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    if (gk_lg2_index_load(repository, "add all paths") != GK_SUCCESS) {
        return gk_session_failure(session);
    }

    char *path_pattern = strdup(pattern);
    git_strarray paths = {&path_pattern, 1};
    int rc = git_index_add_all(session->repository->lg2_resources->index, &paths, GIT_INDEX_ADD_DEFAULT, NULL, NULL);
    gk_lg2_index_free(session);
    free(path_pattern);

    if (rc != 0) {
        return gk_session_failure(session);
    }
    return gk_session_success(session);
}

int gk_repository_index_update_all(gk_session *session, const char* pattern) {
    const char *purpose = "update all paths in index";
    if (gk_session_context_push(session, purpose, &COMP_CONFLICTS, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    if (gk_lg2_index_load(repository, "update all paths") != GK_SUCCESS) {
        return gk_session_failure(session);
    }

    char *path_pattern = strdup(pattern);
    git_strarray paths = {&path_pattern, 1};
    int rc = git_index_update_all(session->repository->lg2_resources->index, &paths, NULL, NULL);
    free(path_pattern);
    gk_lg2_index_free(repository);

    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_repository_lg2_failure_ex(session, purpose, GK_ERR, "failed to update all paths matching [%s]", pattern);
    }

    return gk_session_success(session);
}
