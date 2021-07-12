
#include <string.h>

#include "gk_commit.h"
#include "gk_logging.h"
#include "gk_results.h"
#include "gk_lg2_private.h"
#include "git2.h"

size_t gk_session_count_reflog_entries(gk_session *session, const char* ref_name) {
    gk_result *result = NULL;
    git_index *index = NULL;
    int rc;

    if (gk_session_verify(session, &COMP_COMMIT, GK_SESSION_VERIFY_LOCAL_CHECKOUT, "count reflog entries") != GK_SUCCESS) {
        return 0;
    }

    if (gk_lg2_index_load(session, "count reflog entries") != GK_SUCCESS) {
        return 0;
    }

    if (gk_lg2_reflog_read(session, ref_name, "count reflog entries") != GK_SUCCESS) {
        return 0;
    }

    size_t entrycount = git_reflog_entrycount(session->lg2_resources->reflog);
    gk_lg2_reflog_free(session);
    gk_lg2_index_free(session);

    gk_session_success(session);
    return entrycount;
}

int gk_session_commit(gk_session *session, const char *ref_name, const char* commit_message, gk_object_id *out_commit_id) {
    gk_result *result = NULL;
    git_oid commit_oid,tree_oid;
    git_tree *tree;
    git_index *index;	
    git_object *parent = NULL;
    git_reference *ref = NULL;
    git_signature *signature;
    int rc;
    char message[256] = {0};
        
    if (session == NULL) {
        log_error(COMP_COMMIT, "Cannot commit, session is NULL");
        return GK_FAILURE;
    }

    if (gk_session_state_disabled(session, GK_SESSION_STATE_LOCAL_CHECKOUT_EXISTS)) {
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, local checkout does not exist");
    }
    else if (session->lg2_resources->repository == NULL) {
        // should never happen if local_checkout_exists == 1
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, internal git2 repository is unexpectedely NULL");
    }
    else if (ref_name == NULL) {
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, ref is NULL");
    }

    rc = git_repository_index(&index, session->lg2_resources->repository);
    
    if (rc != 0) {
        git_index_free(index);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, error obtaining repository index (%d) while: %s", err->klass, err->message);
    }

    const char *safe_commit_message = commit_message == NULL ? "" : commit_message;
    
    rc = git_revparse_ext(&parent, &ref, session->lg2_resources->repository, ref_name);
    if ((rc != 0) && (rc != GIT_ENOTFOUND)) {
        git_index_free(index);
        git_object_free(parent);
        git_reference_free(ref);
        // NOTE: ref not found => create the first commit
        //       so only fail if it's a different error
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, error parsing ref '%s' (%d): %s", ref_name, err->klass, err->message);
    }

    rc = git_signature_default(&signature, session->lg2_resources->repository);
    if (rc != 0) {
        git_index_free(index);
        git_object_free(parent);
        git_reference_free(ref);
        git_signature_free(signature);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, error creating signature (%d): %s", err->klass, err->message);
    }

    rc = git_index_write_tree(&tree_oid, index);
    if (rc != 0) {
        git_index_free(index);
        git_object_free(parent);
        git_reference_free(ref);
        git_signature_free(signature);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, error writing index tree (%d): %s", err->klass, err->message);
    }

    rc = git_index_write(index);
    if (rc != 0) {
        git_index_free(index);
        git_object_free(parent);
        git_reference_free(ref);
        git_signature_free(signature);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, error writing index (%d): %s", err->klass, err->message);
    }
    
    rc = git_tree_lookup(&tree, session->lg2_resources->repository, &tree_oid);
    if (rc != 0) {
        git_index_free(index);
        git_object_free(parent);
        git_reference_free(ref);
        git_signature_free(signature);
        git_tree_free(tree);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, error looking up tree (%d): %s", err->klass, err->message);
    }

    rc = git_commit_create_v(&commit_oid, session->lg2_resources->repository, ref_name, signature, signature, NULL, safe_commit_message, tree, parent != NULL? 1 : 0, parent);
    if (rc != 0) {
        git_index_free(index);
        git_object_free(parent);
        git_reference_free(ref);
        git_signature_free(signature);
        git_tree_free(tree);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Error creating commit (%d): %s", err->klass, err->message);
    }

    if (out_commit_id != NULL) {
        git_oid_tostr(out_commit_id->id, 41, &commit_oid);
    }
    return gk_session_success(session);
}

int gk_session_resolve_reference(gk_session *session, const char *ref_name, gk_object_id *object_id) {
    if (session == NULL) {
        log_error(COMP_COMMIT, "Cannot resolve reference, session is NULL");
        return GK_FAILURE;
    }
    if (gk_session_state_disabled(session, GK_SESSION_STATE_LOCAL_CHECKOUT_EXISTS)) {
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot resolve reference, local checkout does not exist");
    }
    else if (session->lg2_resources->repository == NULL) {
        // should never happen if local_checkout_exists == 1
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot resolve reference, internal git2 repository is unexpectedely NULL");
    }
    
    if (ref_name == NULL) {
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot resolve reference, reference name is NULL");
    }
    if (object_id == NULL) {
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot resolve reference, destination object is NULL ");
    }

    git_oid oid;
    int rc = git_reference_name_to_id(&oid, session->lg2_resources->repository, ref_name);
    if (rc == GIT_ENOTFOUND) {
        return gk_session_failure(session, &COMP_COMMIT, -1, "Reference '%s' could not be found", ref_name);
    }
    else if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -2, "Error resolving reference '%s' (%d): %s", ref_name, err->klass, err->message);
    }

    git_oid_tostr(object_id->id, 41, &oid);

    return gk_session_success(session);
}
