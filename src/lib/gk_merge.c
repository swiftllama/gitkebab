
#include "git2.h"

#include "gk_types.h"
#include "gk_logging.h"
#include "gk_session.h"


int gk_session_analyze_merge_into_head(gk_session *session, const char* from_ref_name, int *out_analysis) {
    if (session == NULL) {
        log_error(COMP_MERGE, "Cannot analyze merge, session is NULL");
        return GK_FAILURE;
    }
    if (session->state.local_checkout_exists == 0) {
        return gk_session_failure(session, &COMP_MERGE, -3, "Cannot analyze merge, local checkout does not exist");
    }
    if (session->lg2_repository == NULL) {
        return gk_session_failure(session, &COMP_MERGE, -4, "Cannot analyze merge, internal git2 repository is unexpectedely NULL");
    }
    if (from_ref_name == NULL) {
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot analyze merge from NULL reference");
    }

    git_reference *ref = NULL;
    git_object *parent = NULL;
    int rc = git_revparse_ext(&parent, &ref, session->lg2_repository, from_ref_name);
    if (rc == GIT_ENOTFOUND) {
        git_reference_free(ref);
        git_object_free(parent);
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot analyze merge, reference '%s' not found", from_ref_name);
    }
    if ((rc != 0)) {
        git_object_free(parent);
        git_reference_free(ref);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot analyze merge, the reference '%s' could not be looked up (%d): %s", from_ref_name, err->klass, err->message);
    }
        
    git_annotated_commit *annotated_commit = NULL;
    rc = git_annotated_commit_from_ref(&annotated_commit, session->lg2_repository, ref);
    if ((rc != 0)) {
        git_object_free(parent);
        git_reference_free(ref);
        git_annotated_commit_free(annotated_commit);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot analyze merge, error annotating commit for referencd '%s' (%d): %s", from_ref_name, err->klass, err->message);
    }    

    log_info(COMP_MERGE, "Initiating merge analysis for merging ref '%s' into HEAD in repository '%s'", from_ref_name, session->repository.local_path);
    git_merge_analysis_t analysis;
    git_merge_preference_t preference;
    git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;
    rc = git_merge_analysis(&analysis, &preference, session->lg2_repository, (const git_annotated_commit **)&annotated_commit, 1);
    git_reference_free(ref);
    git_object_free(parent);
    git_annotated_commit_free(annotated_commit);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot analyze merge from reference '%s' (%d): %s", from_ref_name, err->klass, err->message);
    }
    
    log_info(COMP_MERGE, "Merge analysis result is: %d", analysis);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_NORMAL: %d", (analysis & GIT_MERGE_ANALYSIS_NORMAL) != 0 ? GIT_MERGE_ANALYSIS_NORMAL : 0);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_UP_TO_DATE: %d", (analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) != 0 ? GIT_MERGE_ANALYSIS_UP_TO_DATE : 0);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_FASTFORWARD: %d", (analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0 ? GIT_MERGE_ANALYSIS_FASTFORWARD : 0);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_UNBORN: %d", (analysis & GIT_MERGE_ANALYSIS_UNBORN) != 0 ? GIT_MERGE_ANALYSIS_UNBORN : 0);


    session->state.has_changes_to_merge = (analysis != GIT_MERGE_ANALYSIS_UP_TO_DATE);
    log_info(COMP_MERGE, "Set session has changes to merge to %d", session->state.has_changes_to_merge);
    if (analysis == GIT_MERGE_ANALYSIS_UNBORN) {
        log_warn(COMP_MERGE, "Repositor merge analysis resulted UNBORN, this is unexpected");
    }

    if (out_analysis != NULL) {
        *out_analysis = analysis;
    }
    
    return gk_session_success(session);
}

static int merge_fast_forward(gk_session *session, const char *from_ref_name) {
    git_reference *target_ref = NULL;
    git_reference *new_target_ref = NULL;;
    git_object *target = NULL;
    
    int rc  = git_repository_head(&target_ref, session->lg2_repository);
    if (rc != 0) {
        git_reference_free(target_ref);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot perform fast-forward merge, failed obtaining repository head (%d): %s", err->klass, err->message);
    }

    rc = git_revparse_ext(&target, &target_ref, session->lg2_repository, from_ref_name);
    if (rc == GIT_ENOTFOUND) {
        git_object_free(target);
        git_reference_free(target_ref);
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot preform fast-forward merge, reference '%s' not found", from_ref_name);
    }
    if ((rc != 0)) {
        git_object_free(target);
        git_reference_free(target_ref);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot perform fast-forward merge, the reference '%s' could not be looked up (%d): %s", from_ref_name, err->klass, err->message);
    }
    
    gk_authenticated_session authed_session;
    gk_authenticated_session_init(&authed_session, session, NULL);
    
    git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_options.progress_cb = gk_session_checkout_progress_callback;
    checkout_options.progress_payload = &authed_session;
    
    rc = git_checkout_tree(session->lg2_repository, target, &checkout_options);
    if (rc != 0) {
        git_object_free(target);
        git_reference_free(target_ref);        
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -6, "Error preforming fast-forward merge, checkout failed (%d): %s", err->klass, err->message);
    }

    /* Move the target reference to the target OID */
    const git_oid *target_oid = git_object_id(target);
    rc = git_reference_set_target(&new_target_ref, target_ref, target_oid, NULL);
    if (rc != 0) {
        git_object_free(target);
        git_reference_free(target_ref);
        git_reference_free(new_target_ref);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -6, "Error preforming fast-forward merge, failed to advance head to oid '%s' (%d): %s", git_oid_tostr_s(target_oid), err->klass, err->message);
    }


    git_reference_free(target_ref);
    git_reference_free(new_target_ref);
    git_object_free(target);

    return gk_session_success(session);
}


int gk_session_merge_into_head(gk_session *session, const char* from_ref_name) {
    int merge_analysis = 0;
    log_info(COMP_MERGE, "merging '%s' into HEAD", from_ref_name);
    int rc = gk_session_analyze_merge_into_head(session, from_ref_name, &merge_analysis);
    if (rc == GK_FAILURE) {
        return GK_FAILURE;
    }

    if ((merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0) {
        log_info(COMP_MERGE, "will attempt a fast-forward merge");
        rc = merge_fast_forward(session, from_ref_name);
        if (rc == GK_FAILURE) {
            log_info(COMP_MERGE, "Fast-forward merge failed: %s (%d)", gk_result_message(session->last_result), gk_result_code(session->last_result));
            return GK_FAILURE;
        }
    }
    else if ((merge_analysis & GIT_MERGE_ANALYSIS_NORMAL) != 0) {
        // do a real merge
    }
    else if ((merge_analysis & GIT_MERGE_ANALYSIS_UNBORN) != 0) {
        return gk_session_failure(session, &COMP_MERGE, -4, "Error merging changes from server: head points to an unknonw commit id");
    }
    else if ((merge_analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) != 0) {
        log_info(COMP_MERGE, "HEAD is up to date with '%s', no merge is necessary", from_ref_name);
        // nothing to do
    }
    else {
        log_warn(COMP_MERGE, "unknown merge analysis state %d while merging, no merge will be performed", merge_analysis);
    }

    log_info(COMP_MERGE, "merge succeeded");
    return gk_session_success(session);
}
