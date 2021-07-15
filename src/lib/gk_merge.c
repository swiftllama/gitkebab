
#include "git2.h"

#include "gk_types.h"
#include "gk_logging.h"
#include "gk_session.h"
#include "gk_lg2_private.h"

int gk_session_analyze_merge_into_head(gk_session *session, const char* from_ref_name, int *out_analysis) {
    if (gk_session_verify(session, &COMP_MERGE, GK_SESSION_VERIFY_LOCAL_CHECKOUT, "analyze merge") != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (from_ref_name == NULL) {
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot analyze merge from NULL reference");
    }

    log_info(COMP_MERGE, "Initiating merge analysis for merging ref '%s' into HEAD in repository '%s'", from_ref_name, session->repository.local_path);
    git_merge_analysis_t analysis;
    git_merge_preference_t preference;

    int rc = git_merge_analysis(&analysis, &preference, session->lg2_resources->repository, (const git_annotated_commit **)&session->lg2_resources->annotated_fetch_head_commit, 1);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot analyze merge from reference '%s' (%d): %s", from_ref_name, err->klass, err->message);
    }
    
    log_info(COMP_MERGE, "Merge analysis result is: %d", analysis);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_NORMAL: %d", (analysis & GIT_MERGE_ANALYSIS_NORMAL) != 0 ? GIT_MERGE_ANALYSIS_NORMAL : 0);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_UP_TO_DATE: %d", (analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) != 0 ? GIT_MERGE_ANALYSIS_UP_TO_DATE : 0);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_FASTFORWARD: %d", (analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0 ? GIT_MERGE_ANALYSIS_FASTFORWARD : 0);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_UNBORN: %d", (analysis & GIT_MERGE_ANALYSIS_UNBORN) != 0 ? GIT_MERGE_ANALYSIS_UNBORN : 0);


    if (analysis == GIT_MERGE_ANALYSIS_UP_TO_DATE) {
        gk_session_state_unset(session, GK_SESSION_STATE_HAS_CHANGES_TO_MERGE);
    }
    else {
        gk_session_state_set(session, GK_SESSION_STATE_HAS_CHANGES_TO_MERGE);
    }
    log_info(COMP_MERGE, "Set session has changes to merge to %d", gk_session_state_enabled(session, GK_SESSION_STATE_HAS_CHANGES_TO_MERGE));
    if (analysis == GIT_MERGE_ANALYSIS_UNBORN) {
        log_warn(COMP_MERGE, "Repositor merge analysis resulted UNBORN, this is unexpected");
    }

    if (out_analysis != NULL) {
        *out_analysis = analysis;
    }
    
    return gk_session_success(session);
}

static int create_merge_commit(gk_session *session) {
    // Find parents
    if (gk_lg2_parents_lookup(session, "create merge commit") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    if (gk_lg2_signature_create(session, &COMP_MERGE, "create merge commit") != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    git_oid commit_oid;
    int rc = git_commit_create(&commit_oid,
                           session->lg2_resources->repository, git_reference_name(session->lg2_resources->repository_head_ref),
                           session->lg2_resources->signature, session->lg2_resources->signature,
                           NULL, "Merge refs/remotes/origin/master into master",
                           session->lg2_resources->tree,
                           2, (const git_commit **)session->lg2_resources->merge_parents);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot create merge commit, error creating commit (%d): %s", err->klass, err->message);
    }

    log_info(COMP_MERGE, "Created merge commit '%s'", git_oid_tostr_s(&commit_oid));
    git_repository_state_cleanup(session->lg2_resources->repository);

    return GK_SUCCESS;    
}


static int merge_in_memory(gk_session *session) {
    git_merge_options merge_options = GIT_MERGE_OPTIONS_INIT;
    git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;

    merge_options.flags = 0;
    merge_options.file_flags = GIT_MERGE_FILE_STYLE_DIFF3;

    gk_authenticated_session authed_session;
    gk_authenticated_session_init(&authed_session, session, NULL);
    
    checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE | GIT_CHECKOUT_ALLOW_CONFLICTS;
    checkout_options.progress_cb = gk_session_checkout_progress_callback;
    checkout_options.progress_payload = &authed_session;

    gk_lg2_merge_index_free(session);
    int rc = git_merge_commits(&session->lg2_resources->merge_index, session->lg2_resources->repository, session->lg2_resources->repository_head_commit, session->lg2_resources->fetch_head_commit, &merge_options);
    if (rc != 0) {
        gk_lg2_merge_index_free(session);
        const git_error *err = git_error_last();
        gk_session_failure(session, &COMP_MERGE, -5, "Error performing in-memory merge (%d): %s", err->klass, err->message);
        return GK_FAILURE;
    }

    gk_lg2_promote_merge_index(session);
    if (git_index_has_conflicts(session->lg2_resources->index) == 0) {
        if (gk_lg2_index_write_tree(session, session->lg2_resources->index, "merge in memory") != GK_SUCCESS) {
            return GK_FAILURE;
        }
        
        if (gk_lg2_checkout_tree(session, &checkout_options, "merge in memory") != GK_SUCCESS) {
            return GK_FAILURE;
        }

        rc = create_merge_commit(session);
        gk_lg2_merge_index_free(session);
        if (rc != GK_SUCCESS) {
            return GK_FAILURE;
        }
        gk_session_state_unset(session, GK_SESSION_STATE_HAS_CONFLICTS);
        gk_session_state_unset(session, GK_SESSION_STATE_HAS_CHANGES_TO_MERGE);
    }
    else { // has conflicts
        log_error(COMP_MERGE, "detected conflicts after merge");
        if (gk_lg2_iterate_conflicts(session, "merge in memory") != GK_SUCCESS) {;
            return GK_FAILURE;
        }
        gk_session_state_set(session, GK_SESSION_STATE_HAS_CONFLICTS);
        gk_session_state_set(session, GK_SESSION_STATE_HAS_CHANGES_TO_MERGE);
    }

    return GK_SUCCESS;
}


/*
static int merge_normal(gk_session *session) {    
    git_merge_options merge_options = GIT_MERGE_OPTIONS_INIT;
    git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;

    merge_options.flags = 0;
    merge_options.file_flags = GIT_MERGE_FILE_STYLE_DIFF3;

    gk_authenticated_session authed_session;
    gk_authenticated_session_init(&authed_session, session, NULL);
    
    checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE | GIT_CHECKOUT_ALLOW_CONFLICTS;
    checkout_options.progress_cb = gk_session_checkout_progress_callback;
    checkout_options.progress_payload = &authed_session;
    
    int rc = git_merge(session->lg2_resources->repository, (const git_annotated_commit **)&session->lg2_resources->annotated_fetch_head_commit, 1, &merge_options, &checkout_options);
    if (rc != 0) {
        const git_error *err = git_error_last();
        gk_session_failure(session, &COMP_MERGE, -5, "Error performing normal merge (%d): %s", err->klass, err->message);
        return GK_FAILURE;
    }

    rc = gk_lg2_index_load(session, "perform normal merge");
    if (rc != GK_SUCCESS) {
        return GK_FAILURE;
    }

    if (git_index_has_conflicts(session->lg2_resources->index) == 1) {
        log_info(COMP_MERGE, "Encountered conflicts after normal merge, setting state to GK_SESSION_STATE_MERGE_FINALIZATION_PENDING");
        gk_session_state_set(session, GK_SESSION_STATE_MERGE_FINALIZATION_PENDING | GK_SESSION_STATE_HAS_CONFLICTS);
    }
    else {
        if (gk_lg2_index_write_tree(session, session->lg2_resources->index, "peroform normal merge") != GK_SUCCESS) {
            return GK_FAILURE;
        }
        rc = create_merge_commit(session);
        if (rc != GK_SUCCESS) {
            return GK_FAILURE;
        }
    }

    return gk_session_success(session);
    }*/

static int merge_fast_forward(gk_session *session, const char *from_ref_name) {    
    log_info(COMP_MERGE, "Fast-forwardig merge from current HEAD at '%s' to %s at '%s'", session->lg2_resources->repository_head_oid_id , from_ref_name, session->lg2_resources->fetch_head_oid_id);
    
    gk_authenticated_session authed_session;
    gk_authenticated_session_init(&authed_session, session, NULL);
    
    git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_options.progress_cb = gk_session_checkout_progress_callback;
    checkout_options.progress_payload = &authed_session;

    log_debug(COMP_MERGE, "Checked out tree at rev '%s' into working directory", session->lg2_resources->fetch_head_oid_id);
    int rc = git_checkout_tree(session->lg2_resources->repository, session->lg2_resources->fetch_head_object, &checkout_options);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -6, "Error preforming fast-forward merge, checkout failed (%d): %s", err->klass, err->message);
    }

    git_reference *new_head_ref = NULL;
    log_debug(COMP_MERGE, "Advancing HEAD to rev '%s'", session->lg2_resources->fetch_head_oid_id);
    /* Move the target reference to the target OID */
    rc = git_reference_set_target(&new_head_ref, session->lg2_resources->repository_head_ref, session->lg2_resources->fetch_head_oid, NULL);
    if (rc != 0) {
        const git_error *err = git_error_last();
        git_reference_free(new_head_ref);
        return gk_session_failure(session, &COMP_MERGE, -6, "Error preforming fast-forward merge, failed to advance head to oid '%s' (%d): %s", session->lg2_resources->fetch_head_oid_id, err->klass, err->message);
    }

    gk_session_state_unset(session, GK_SESSION_STATE_HAS_CONFLICTS);
    gk_session_state_unset(session, GK_SESSION_STATE_HAS_CHANGES_TO_MERGE);
    
    return gk_session_success(session);
}

int gk_session_merge_into_head(gk_session *session, const char* from_ref_name) {
    int merge_analysis = 0;
    log_info(COMP_MERGE, "merging '%s' into HEAD", from_ref_name);
    gk_session_state_set(session, GK_SESSION_STATE_MERGE_IN_PROGRESS);

    if (gk_lg2_load_references(session, from_ref_name, "analyze merge for fetch") != GK_SUCCESS) {
        gk_lg2_free_all_but_repository(session);
        return GK_FAILURE;
    }

    if (gk_session_state_enabled(session, GK_SESSION_STATE_MERGE_PENDING_ON_DISK)) {
        return gk_session_failure(session, &COMP_MERGE, -5, "UNIMPLEMENTED: merge pending on disk");
    }
    
    int rc = gk_session_analyze_merge_into_head(session, from_ref_name, &merge_analysis);
    if (rc == GK_FAILURE) {
        gk_session_state_unset(session, GK_SESSION_STATE_MERGE_IN_PROGRESS);
        gk_lg2_free_all_but_repository(session);
        return GK_FAILURE;
    }

    if ((merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0) {
        log_info(COMP_MERGE, "will attempt a fast-forward merge");
        rc = merge_fast_forward(session, from_ref_name);
        if (rc == GK_FAILURE) {
            log_info(COMP_MERGE, "Fast-forward merge failed: %s (%d)", gk_result_message(session->last_result), gk_result_code(session->last_result));
            gk_session_state_unset(session, GK_SESSION_STATE_MERGE_IN_PROGRESS);
            gk_lg2_free_all_but_repository(session);
            return GK_FAILURE;
        }
    }
    else if ((merge_analysis & GIT_MERGE_ANALYSIS_NORMAL) != 0) {
        log_info(COMP_MERGE, "will attempt an in-memory merge");
        rc = merge_in_memory(session);
        if (rc == GK_FAILURE) {
            log_info(COMP_MERGE, "in-memory merge failed: %s (%d)", gk_result_message(session->last_result), gk_result_code(session->last_result));
            gk_session_state_unset(session, GK_SESSION_STATE_MERGE_IN_PROGRESS);
            gk_lg2_free_all_but_repository(session);
            return GK_FAILURE;
        }
    }
    else if ((merge_analysis & GIT_MERGE_ANALYSIS_UNBORN) != 0) {
        gk_session_state_unset(session, GK_SESSION_STATE_MERGE_IN_PROGRESS);
        gk_lg2_free_all_but_repository(session);
        return gk_session_failure(session, &COMP_MERGE, -4, "Error merging changes from server: head points to an unknonw commit id");
    }
    else if ((merge_analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) != 0) {
        log_info(COMP_MERGE, "HEAD is up to date with '%s', no merge is necessary", from_ref_name);
        // nothing to do
    }
    else {
        log_warn(COMP_MERGE, "unknown merge analysis state %d while merging, no merge will be performed", merge_analysis);
    }

    gk_session_state_unset(session, GK_SESSION_STATE_MERGE_IN_PROGRESS);

    if (gk_session_state_enabled(session, GK_SESSION_STATE_HAS_CONFLICTS)) {
        log_info(COMP_MERGE, "merge attemt ended with conflicts");
    }
    else if (gk_session_state_enabled(session, GK_SESSION_STATE_HAS_CHANGES_TO_MERGE)) {
        log_info(COMP_MERGE, "merge attemt ended without conflicts but merge still pending");
    }
    else {
        log_info(COMP_MERGE, "merge attemt ended, all changes merged");
    }

    gk_lg2_free_all_but_repository(session);
    return gk_session_success(session);
}
