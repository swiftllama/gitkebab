
#include <string.h>

#include "gk_session.h"
#include "gk_internal_resources_private.h"
#include "gk_logging.h"

gk_internal_resources *gk_internal_resources_new() {
    gk_internal_resources *resources = (gk_internal_resources *)malloc(sizeof(gk_internal_resources));
    if (resources == NULL) {
        log_error(COMP_GENERAL, "Error allocating internal resources struct");
        return NULL;
    }
    memset(resources, 0, sizeof(gk_internal_resources));
    return resources;
}

void gk_internal_resources_free(gk_internal_resources *resources) {
    if (resources == NULL) {
        return;
    }
    gk_internal_resources_free_references(resources);
    gk_internal_resources_free_index(resources);

    git_tree_free(resources->tree);
    git_signature_free(resources->signature);
    free(resources->merge_parents);

    resources->tree = NULL;
    resources->signature = NULL;
    resources->merge_parents = NULL;

    free(resources);
}

void gk_internal_resources_free_references(gk_internal_resources *resources) {
    git_reference_free(resources->repository_head_ref);
    git_object_free(resources->repository_head_object);
    
    git_reference_free(resources->fetch_head_ref);
    git_object_free(resources->fetch_head_object);
    git_annotated_commit_free(resources->annotated_fetch_head_commit);

    resources->repository_head_ref = NULL;
    resources->repository_head_object = NULL;
    resources->fetch_head_ref = NULL;
    resources->fetch_head_object = NULL;
    resources->annotated_fetch_head_commit = NULL;
}

int gk_internal_resources_load_references(gk_session *session, gk_internal_resources *resources, const char *from_ref_name, const char *purpose) {
    if (session == NULL) {
        log_error(COMP_MERGE, "Cannot load resources references for NULL session");
        return GK_FAILURE;
    }

    int rc = git_revparse_ext(&resources->fetch_head_object, &resources->fetch_head_ref, session->lg2_repository, from_ref_name);
    if (rc == GIT_ENOTFOUND) {
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot %s, reference '%s' not found", purpose, from_ref_name);
    }
    if ((rc != 0)) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot %s, the reference '%s' could not be looked up (%d): %s", purpose, from_ref_name, err->klass, err->message);
    }

    rc = git_revparse_ext(&resources->repository_head_object, &resources->repository_head_ref, session->lg2_repository, "HEAD");
    if (rc == GIT_ENOTFOUND) {
        return gk_session_failure(session, &COMP_MERGE, -5, "cannot %s, reference HEAD not found", purpose);
    }
    if ((rc != 0)) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "cannot %s, failed to resolve HEAD reference (%d): %s", purpose, err->klass, err->message);
    }

    rc = git_annotated_commit_from_ref(&resources->annotated_fetch_head_commit, session->lg2_repository, resources->fetch_head_ref);
    if ((rc != 0)) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot %s, error annotating commit for referencd '%s' (%d): %s", purpose, from_ref_name, err->klass, err->message);
    }    

    resources->repository_head_oid = git_object_id(resources->repository_head_object);
    resources->fetch_head_oid = git_object_id(resources->fetch_head_object);
    git_oid_tostr(resources->repository_head_oid_id, 41, resources->repository_head_oid);
    git_oid_tostr(resources->fetch_head_oid_id, 41, resources->fetch_head_oid);
    
    return GK_SUCCESS;
}

int gk_internal_resources_load_index(gk_session *session, gk_internal_resources *resources, const char *purpose) {
    if (session == NULL) {
        log_error(COMP_MERGE, "Cannot load resources references for NULL session");
        return GK_FAILURE;
    }

    int rc = git_repository_index(&resources->index, session->lg2_repository);
    if (rc != 0) {
        gk_internal_resources_free_index(resources);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -6, "Error %s, failed to retrieve repository index (%d): %s", purpose, err->klass, err->message);
    }

    return GK_SUCCESS;
}

void gk_internal_resources_free_index(gk_internal_resources *resources) {
    if (resources == NULL) {
        return;
    }

    git_index_free(resources->index);
    resources->index = NULL;
}
