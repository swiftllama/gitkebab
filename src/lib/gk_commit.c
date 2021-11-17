
#include <string.h>
#include <time.h>
#include "git2.h"

#include "gk_commit.h"
#include "gk_logging.h"
#include "gk_results.h"
#include "gk_lg2_private.h"
#include "gk_session.h"

size_t gk_count_reflog_entries(gk_session *session, const char* ref_name) {
    const char *purpose = "count reflog entries";
    if (gk_session_context_push(session, purpose, &COMP_COMMIT, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return 0;
    }
    
    if (gk_lg2_index_load(session) != GK_SUCCESS) {
        gk_session_failure(session, purpose);
        return 0;
    }

    if (gk_lg2_reflog_read(session, ref_name) != GK_SUCCESS) {
        gk_session_failure(session, purpose);
        return 0;
    }

    size_t entrycount = git_reflog_entrycount(session->repository->lg2_resources->reflog);
    gk_lg2_reflog_free(session->repository);
    gk_lg2_index_free(session->repository);

    gk_session_success(session, purpose);
    return entrycount;
}

int gk_commit(gk_session *session, const char* commit_message, gk_object_id *out_commit_id) {
    const char *purpose = "commit changes";
    if (gk_session_context_push(session, purpose, NULL, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    const char *safe_commit_message = commit_message == NULL ? "" : commit_message;

    if (gk_lg2_load_references(session) != GK_SUCCESS) {
        gk_lg2_free_references(session->repository);
        return gk_session_failure(session, purpose);
    }

    if (gk_lg2_index_load(session) != GK_SUCCESS) {
        gk_lg2_free_references(session->repository);
        gk_lg2_index_free(session->repository);
        return gk_session_failure(session, purpose);
    }
    
    if (gk_lg2_signature_create(session) != GK_SUCCESS) {
        gk_lg2_free_references(session->repository);
        gk_lg2_index_free(session->repository);
        return gk_session_failure(session, purpose);
    }

    if (gk_lg2_index_write_tree(session, lg2_resources->index) != GK_SUCCESS) {
        gk_lg2_free_references(session->repository);
        gk_lg2_signature_free(session->repository);
        gk_lg2_index_free(session->repository);
        return gk_session_failure(session, purpose);
    }

    if (git_index_write(lg2_resources->index) != 0) {
        gk_lg2_free_references(session->repository);
        gk_lg2_signature_free(session->repository);        
        gk_lg2_tree_free(session->repository);
        gk_lg2_index_free(session->repository);
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to write index");
    }

    git_oid commit_oid;
    if (git_commit_create_v(&commit_oid, lg2_resources->repository, "HEAD", lg2_resources->signature, lg2_resources->signature, NULL, safe_commit_message, lg2_resources->tree, lg2_resources->repository_head_object != NULL? 1 : 0, lg2_resources->repository_head_object) != 0) {
        gk_lg2_free_references(session->repository);
        gk_lg2_signature_free(session->repository);
        gk_lg2_tree_free(session->repository);
        gk_lg2_index_free(session->repository);
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to create commit");
    }

    if (out_commit_id != NULL) {
        git_oid_tostr(out_commit_id->id, 41, &commit_oid);
    }
    log_info(COMP_COMMIT, "Created commit [%s]", git_oid_tostr_s(&commit_oid));
    
    return gk_session_success(session, purpose);
}

int gk_resolve_reference(gk_session *session, const char *ref_name, gk_object_id *object_id) {
    const char *purpose = "resolve reference";
    if (gk_session_context_push(session, purpose, NULL, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    if (ref_name == NULL) {
        return gk_session_failure_ex(session, purpose, GK_ERR, "reference name is NULL");
    }
    if (object_id == NULL) {
        return gk_session_failure_ex(session, purpose, GK_ERR, "destination object is NULL");
    }

    git_oid oid;
    int rc = git_reference_name_to_id(&oid, session->repository->lg2_resources->repository, ref_name);
    if (rc == GIT_ENOTFOUND) {
        return gk_session_failure_ex(session, purpose, GK_ERR, "reference [%s] could not be found", ref_name);
    }
    else if (rc != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to find reference name [%s]", ref_name);
    }

    git_oid_tostr(object_id->id, 41, &oid);

    return gk_session_success(session, purpose);
}

const char * gk_object_id_ptr(gk_object_id *object_id) {
    return object_id->id;
}
