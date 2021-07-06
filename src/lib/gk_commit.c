
#include <string.h>

#include "gk_commit.h"
#include "gk_logging.h"
#include "gk_results.h"
#include "git2.h"

size_t gk_session_count_reflog_entries(gk_session_t *session, const char* ref_name) {
    gk_result_t *result = NULL;
    git_index *index = NULL;
    int rc;
    
    if (session == NULL) {
        log_error(COMP_COMMIT, "Cannot count reflog entries, session is NULL");
        return 0;
    }
    if (session->state.local_checkout_exists == 0) {
        gk_session_failure(session, &COMP_COMMIT, -3, "Cannot count reflog entries, local checkout does not exist");
        return 0;
    }
    else if (session->lg2_repository == NULL) {
        // should never happen if local_checkout_exists == 1
        gk_session_failure(session, &COMP_COMMIT, -3, "Cannot count reflog entries, internal git2 repository is unexpectedely NULL");
        return 0;
    }

    rc = git_repository_index(&index, session->lg2_repository);
    if (rc != 0) {
        const git_error *err = git_error_last();
        gk_session_failure(session, &COMP_COMMIT, -3, "Cannot count reflog entries, error obtaining repository index (%d) while: %s", err->klass, err->message);
        git_index_free(index);
        return 0;
    }

    git_reflog *reflog = NULL;
    rc = git_reflog_read(&reflog, session->lg2_repository, ref_name);
    size_t entrycount = rc == 0 ? git_reflog_entrycount(reflog) : 0;
    git_index_free(index);
    git_reflog_free(reflog);
    
    if (rc != 0) {
        const git_error *err = git_error_last();
        gk_session_failure(session, &COMP_COMMIT, -3, "Error counting reflog entries (%d): %s", err->klass, err->message);
    }
    else {
        gk_session_success(session);
    }

    return entrycount;
}

int gk_session_commit(gk_session_t *session, const char *ref_name, const char* commit_message, gk_object_id_t *out_commit_id) {
    gk_result_t *result = NULL;
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

    if (session->state.local_checkout_exists == 0) {
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, local checkout does not exist");
    }
    else if (session->lg2_repository == NULL) {
        // should never happen if local_checkout_exists == 1
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, internal git2 repository is unexpectedely NULL");
    }
    else if (ref_name == NULL) {
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, ref is NULL");
    }

    rc = git_repository_index(&index, session->lg2_repository);
    
    if (rc != 0) {
        git_index_free(index);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, error obtaining repository index (%d) while: %s", err->klass, err->message);
    }

    const char *safe_commit_message = commit_message == NULL ? "" : commit_message;
    
    rc = git_revparse_ext(&parent, &ref, session->lg2_repository, ref_name);
    if ((rc != 0) && (rc != GIT_ENOTFOUND)) {
        git_index_free(index);
        git_object_free(parent);
        git_reference_free(ref);
        // NOTE: ref not found => create the first commit
        //       so only fail if it's a different error
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, error parsing ref '%s' (%d): %s", ref_name, err->klass, err->message);
    }

    rc = git_signature_default(&signature, session->lg2_repository);
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
    
    rc = git_tree_lookup(&tree, session->lg2_repository, &tree_oid);
    if (rc != 0) {
        git_index_free(index);
        git_object_free(parent);
        git_reference_free(ref);
        git_signature_free(signature);
        git_tree_free(tree);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot commit, error looking up tree (%d): %s", err->klass, err->message);
    }

    rc = git_commit_create_v(&commit_oid, session->lg2_repository, ref_name, signature, signature, NULL, safe_commit_message, tree, parent != NULL? 1 : 0, parent);
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
        memcpy(out_commit_id->id, commit_oid.id, 20);
    }
    return gk_session_success(session);
}

char *gk_object_id_hex_string_new(gk_object_id_t *object_id) {
    if (object_id == NULL) {
        return NULL;
    }
    char *hex_id = (char *)malloc(GK_OBJECT_ID_LENGTH*2+1);
    if (hex_id == NULL) {
        printf("Error allocating 40 bytes for hex id");
        return NULL;
    }
    for (int i = 0; i <= GK_OBJECT_ID_LENGTH; i += 1) {
        printf("printing byte #%d: %02x\n", i, (unsigned int)object_id->id[i]);
        sprintf(hex_id + i*2, "%02x", (unsigned int)object_id->id[i]);
    }
    hex_id[GK_OBJECT_ID_LENGTH*2] = '\0';
    return hex_id;
}

int gk_session_resolve_reference(gk_session_t *session, const char *ref_name, gk_object_id_t *object_id) {
    if (session == NULL) {
        log_error(COMP_COMMIT, "Cannot resolve reference, session is NULL");
        return GK_FAILURE;
    }
    if (session->state.local_checkout_exists == 0) {
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot resolve reference, local checkout does not exist");
    }
    else if (session->lg2_repository == NULL) {
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
    int rc = git_reference_name_to_id(&oid, session->lg2_repository, ref_name);
    if (rc == GIT_ENOTFOUND) {
        return gk_session_failure(session, &COMP_COMMIT, -1, "Reference '%s' could not be found", ref_name);
    }
    else if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -2, "Error resolving reference '%s' (%d): %s", ref_name, err->klass, err->message);
    }

    memcpy(oid.id, object_id->id, 20);

    return gk_session_success(session);
}
