
#include "git2.h"

#include "gk_types.h"
#include "gk_logging.h"
#include "gk_session.h"

int gk_analyze_merge_into_head(gk_session *session, const char* from_ref_name) {
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

    return gk_session_success(session);
}

