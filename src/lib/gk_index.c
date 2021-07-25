
#include <string.h>

#include "git2.h"
#include "gk_index.h"
#include "gk_results.h"
#include "gk_logging.h"
#include "gk_lg2_private.h"

static int verify_repository_and_path(gk_repository *repository, const char* purpose, const char *path) {
    if (gk_repository_verify(repository, &COMP_COMMIT, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (path == NULL) {
        return gk_repository_failure(repository, &COMP_COMMIT, -3, "Cannot %s, path/pattern is NULL", purpose);
    }
    else if (path[0] == '\0') {
        return gk_repository_failure(repository, &COMP_COMMIT, -3, "Cannot %s, path/pattern is empty", purpose);
    }
    return GK_SUCCESS;
}

int gk_repository_index_add_path(gk_repository *repository, const char *path) {
    if (verify_repository_and_path(repository, "add path to index", path) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    if (gk_lg2_index_load(repository, "add path to index") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    int rc = git_index_add_bypath(repository->lg2_resources->index, path);
    gk_lg2_index_free(repository);
    
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_repository_failure(repository, &COMP_COMMIT, -3, "Error adding path '%s' to repository index (%d): %s", path, err->klass, err->message);
    }
    
    return gk_repository_success(repository);
}

int gk_repository_index_remove_path(gk_repository *repository, const char *path) {
    if (verify_repository_and_path(repository, "remove path from index", path) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    if (gk_lg2_index_load(repository, "remove path from index") != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    int rc = git_index_remove_bypath(repository->lg2_resources->index, path);
    gk_lg2_index_free(repository);

    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_repository_failure(repository, &COMP_COMMIT, -3, "Error removing path '%s' from repository index (%d): %s", path, err->klass, err->message);
    }
    
    return gk_repository_success(repository);
}

int gk_repository_index_add_all(gk_repository *repository, const char* pattern) {
    if (verify_repository_and_path(repository, "add all paths", pattern) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    if (gk_lg2_index_load(repository, "add all paths") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    char *path_pattern = strdup(pattern);
    git_strarray paths = {&path_pattern, 1};
    int rc = git_index_add_all(repository->lg2_resources->index, &paths, GIT_INDEX_ADD_DEFAULT, NULL, NULL);
    free(path_pattern);
    gk_lg2_index_free(repository);

    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_repository_failure(repository, &COMP_COMMIT, -3, "Error adding all paths matching '%s' to repository index (%d): %s", pattern, err->klass, err->message);
    }

    return gk_repository_success(repository);
}

int gk_repository_index_update_all(gk_repository *repository, const char* pattern) {
    if (verify_repository_and_path(repository, "update all paths", pattern) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    if (gk_lg2_index_load(repository, "update all paths") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    char *path_pattern = strdup(pattern);
    git_strarray paths = {&path_pattern, 1};
    int rc = git_index_update_all(repository->lg2_resources->index, &paths, NULL, NULL);
    free(path_pattern);
    gk_lg2_index_free(repository);

    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_repository_failure(repository, &COMP_COMMIT, -3, "Error updating all paths matching '%s' to repository index (%d): %s", pattern, err->klass, err->message);
    }

    return gk_repository_success(repository);
}
