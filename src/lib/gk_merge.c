
#include "git2.h"

#include "gk_types.h"
#include "gk_logging.h"
#include "gk_session.h"
#include "gk_internal_resources_private.h"

int gk_session_analyze_merge_into_head(gk_session *session, const char* from_ref_name, int *out_analysis) {
    if (session == NULL) {
        log_error(COMP_MERGE, "Cannot analyze merge, session is NULL");
        return GK_FAILURE;
    }
    if (gk_session_state_disabled(session, GK_SESSION_STATE_LOCAL_CHECKOUT_EXISTS)) {
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

static int create_merge_commit(gk_session *session, git_index *index, git_reference *fetch_head_ref, git_object *fetch_head_object) {
    git_reference *head_ref = NULL;
    git_object *head_object = NULL;
    git_commit **parents = calloc(2, sizeof(git_commit *));
    
    int rc = git_revparse_ext(&head_object, &head_ref, session->lg2_repository, "HEAD");
    if (rc == GIT_ENOTFOUND) {
        git_reference_free(head_ref);
        git_object_free(head_object);
        free(parents);
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot create merge commit, HEAD not found");
    }
    if ((rc != 0)) {
        git_reference_free(fetch_head_ref);
        git_object_free(fetch_head_object);
        free(parents);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot create merge commit, HEAD could not be looked up (%d): %s", err->klass, err->message);
    }

    // Find parents
    const git_oid *fetch_head_oid = git_object_id(fetch_head_object);
    rc = git_reference_peel((git_object **)&parents[0], head_ref, GIT_OBJECT_COMMIT);
    if (rc != 0) {
        git_reference_free(fetch_head_ref);
        git_object_free(fetch_head_object);
        git_object_free((git_object *)parents[0]);
        free(parents);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot create merge commit, error peeling head reference (%d): %s", err->klass, err->message);
    }

    rc = git_commit_lookup(&parents[1], session->lg2_repository, fetch_head_oid);
    if (rc != 0) {
        git_reference_free(fetch_head_ref);
        git_object_free(fetch_head_object);
        git_object_free((git_object *)parents[0]);
        git_commit_free(parents[1]);
        free(parents);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot create merge commit, error peeling head reference (%d): %s", err->klass, err->message);
    }
    

    git_oid tree_oid;
    rc = git_index_write_tree(&tree_oid, index);
    if (rc != 0) {
        git_reference_free(head_ref);
        git_object_free(head_object);
        git_object_free((git_object *)parents[0]);
        git_commit_free(parents[1]);
        free(parents);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot create merge commit, error writing index tree (%d): %s", err->klass, err->message);
    }

    git_tree *tree = NULL;    
    rc = git_tree_lookup(&tree, session->lg2_repository, &tree_oid);
    if (rc != 0) {
        git_reference_free(head_ref);
        git_object_free(head_object);
        git_tree_free(tree);
        git_object_free((git_object *)parents[0]);
        git_commit_free(parents[1]);
        free(parents);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot create merge commit, error looking up tree (%d): %s", err->klass, err->message);
    }

    git_signature *signature = NULL;
    rc = git_signature_default(&signature, session->lg2_repository);
    if (rc != 0) {
        git_reference_free(head_ref);
        git_object_free(head_object);
        git_tree_free(tree);
        git_object_free((git_object *)parents[0]);
        git_commit_free(parents[1]);
        git_signature_free(signature);
        free(parents);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot create merge commit, error creating signature (%d): %s", err->klass, err->message);
    }
    
    git_oid commit_oid;
    rc = git_commit_create(&commit_oid,
                           session->lg2_repository, git_reference_name(head_ref),
                           signature, signature,
                           NULL, "Merge refs/remotes/origin/master into master",
                           tree,
                           2, (const git_commit **)parents);
    if (rc != 0) {
        git_reference_free(head_ref);
        git_object_free(head_object);
        git_tree_free(tree);
        git_object_free((git_object *)parents[0]);
        git_commit_free(parents[1]);
        git_signature_free(signature);
        free(parents);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot create merge commit, error looking up tree (%d): %s", err->klass, err->message);
    }

    log_info(COMP_MERGE, "Created merge commit '%s'", git_oid_tostr_s(&commit_oid));
    git_repository_state_cleanup(session->lg2_repository);

    git_reference_free(head_ref);
    git_object_free(head_object);
    git_object_free((git_object *)parents[0]);
    git_commit_free(parents[1]);
    git_signature_free(signature);
    free(parents);
    git_tree_free(tree);
    return GK_SUCCESS;
    
}


static int merge_normal(gk_session *session, const char *from_ref_name) {
    git_merge_options merge_options = GIT_MERGE_OPTIONS_INIT;
    git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;

    merge_options.flags = 0;
    merge_options.file_flags = GIT_MERGE_FILE_STYLE_DIFF3;

    gk_authenticated_session authed_session;
    gk_authenticated_session_init(&authed_session, session, NULL);
    
    checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE | GIT_CHECKOUT_ALLOW_CONFLICTS;
    checkout_options.progress_cb = gk_session_checkout_progress_callback;
    checkout_options.progress_payload = &authed_session;

    git_reference *fetch_head_ref = NULL;
    git_object *fetch_head_object = NULL;
    int rc = git_revparse_ext(&fetch_head_object, &fetch_head_ref, session->lg2_repository, from_ref_name);
    if (rc == GIT_ENOTFOUND) {
        git_reference_free(fetch_head_ref);
        git_object_free(fetch_head_object);
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot perform normal merge, reference '%s' not found", from_ref_name);
    }
    if ((rc != 0)) {
        git_reference_free(fetch_head_ref);
        git_object_free(fetch_head_object);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot perform normal merge, the reference '%s' could not be looked up (%d): %s", from_ref_name, err->klass, err->message);
    }
    
    git_annotated_commit *annotated_fetch_head_commit = NULL;
    rc = git_annotated_commit_from_ref(&annotated_fetch_head_commit, session->lg2_repository, fetch_head_ref);
    if ((rc != 0)) {
        git_reference_free(fetch_head_ref);
        git_object_free(fetch_head_object);
        git_annotated_commit_free(annotated_fetch_head_commit);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot perform normal merge, error annotating commit for referencd '%s' (%d): %s", from_ref_name, err->klass, err->message);
    }    
    
    rc = git_merge(session->lg2_repository, (const git_annotated_commit **)&annotated_fetch_head_commit, 1, &merge_options, &checkout_options);
    if (rc != 0) {
        git_reference_free(fetch_head_ref);
        git_object_free(fetch_head_object);
        git_annotated_commit_free(annotated_fetch_head_commit);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Error performing normal merge (%d): %s", from_ref_name, err->klass, err->message);
    }

    // Check index for conflicts
    git_index *index;
    rc = git_repository_index(&index, session->lg2_repository);
    if (rc != 0) {
        git_reference_free(fetch_head_ref);
        git_object_free(fetch_head_object);
        git_index_free(index);
        git_annotated_commit_free(annotated_fetch_head_commit);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -6, "Error performing normal merge, failed to retrieve repository index (%d): %s", err->klass, err->message);
    }

    if (git_index_has_conflicts(index) == 1) {
        log_info(COMP_MERGE, "Encountered conflicts after normal merge");
        // TODO.. handle conflicts
    }
    else {
        rc = create_merge_commit(session, index, fetch_head_ref, fetch_head_object);
        if (rc != 0) {
            git_reference_free(fetch_head_ref);
            git_object_free(fetch_head_object);
            git_annotated_commit_free(annotated_fetch_head_commit);
            git_index_free(index);            
            const git_error *err = git_error_last();
            return gk_session_failure(session, &COMP_MERGE, -6, "Error performing normal merge, failed tocreate merge commit (%d): %s", err->klass, err->message);            
        }
    }

    git_reference_free(fetch_head_ref);
    git_object_free(fetch_head_object);
    git_annotated_commit_free(annotated_fetch_head_commit);
    git_index_free(index);
    
    return gk_session_success(session);
}

static int merge_fast_forward(gk_session *session, const char *from_ref_name) {
    gk_internal_resources *resources = gk_internal_resources_new();
    int rc = gk_internal_resources_load_references(session, resources, from_ref_name, "perform fast forward merge");
    if (rc != GK_SUCCESS) {
        gk_internal_resources_free(resources);
        return GK_FAILURE;
    }
    
    log_info(COMP_MERGE, "Fast-forwardig merge from current HEAD at '%s' to %s at '%s'", resources->repository_head_oid_id , from_ref_name, resources->fetch_head_oid_id);
    
    gk_authenticated_session authed_session;
    gk_authenticated_session_init(&authed_session, session, NULL);
    
    git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_options.progress_cb = gk_session_checkout_progress_callback;
    checkout_options.progress_payload = &authed_session;

    log_debug(COMP_MERGE, "Checked out tree at rev '%s' into working directory", resources->fetch_head_oid_id);
    rc = git_checkout_tree(session->lg2_repository, resources->fetch_head_object, &checkout_options);
    if (rc != 0) {
        gk_internal_resources_free(resources);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -6, "Error preforming fast-forward merge, checkout failed (%d): %s", err->klass, err->message);
    }


    git_reference *new_head_ref = NULL;
    log_debug(COMP_MERGE, "Advancing HEAD to rev '%s'", resources->fetch_head_oid_id);
    /* Move the target reference to the target OID */
    rc = git_reference_set_target(&new_head_ref, resources->repository_head_ref, resources->fetch_head_oid, NULL);
    if (rc != 0) {
        const git_error *err = git_error_last();
        gk_session_failure(session, &COMP_MERGE, -6, "Error preforming fast-forward merge, failed to advance head to oid '%s' (%d): %s", resources->fetch_head_oid_id, err->klass, err->message);
        git_reference_free(new_head_ref);
        gk_internal_resources_free(resources);
        return GK_FAILURE;
    }

    git_reference_free(new_head_ref);
    gk_internal_resources_free(resources);
    return gk_session_success(session);
}


int gk_session_merge_into_head(gk_session *session, const char* from_ref_name) {
    int merge_analysis = 0;
    log_info(COMP_MERGE, "merging '%s' into HEAD", from_ref_name);
    gk_session_state_set(session, GK_SESSION_STATE_MERGE_IN_PROGRESS);
    
    int rc = gk_session_analyze_merge_into_head(session, from_ref_name, &merge_analysis);
    if (rc == GK_FAILURE) {
        gk_session_state_unset(session, GK_SESSION_STATE_MERGE_IN_PROGRESS);
        return GK_FAILURE;
    }

    if ((merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0) {
        log_info(COMP_MERGE, "will attempt a fast-forward merge");
        rc = merge_fast_forward(session, from_ref_name);
        if (rc == GK_FAILURE) {
            log_info(COMP_MERGE, "Fast-forward merge failed: %s (%d)", gk_result_message(session->last_result), gk_result_code(session->last_result));
            gk_session_state_unset(session, GK_SESSION_STATE_MERGE_IN_PROGRESS);
            return GK_FAILURE;
        }
    }
    else if ((merge_analysis & GIT_MERGE_ANALYSIS_NORMAL) != 0) {
        log_info(COMP_MERGE, "will attempt a normal merge");
        rc = merge_normal(session, from_ref_name);
        if (rc == GK_FAILURE) {
            log_info(COMP_MERGE, "normal merge failed: %s (%d)", gk_result_message(session->last_result), gk_result_code(session->last_result));
            gk_session_state_unset(session, GK_SESSION_STATE_MERGE_IN_PROGRESS);
            return GK_FAILURE;
        }
    }
    else if ((merge_analysis & GIT_MERGE_ANALYSIS_UNBORN) != 0) {
        gk_session_state_unset(session, GK_SESSION_STATE_MERGE_IN_PROGRESS);
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
    gk_session_state_unset(session, GK_SESSION_STATE_HAS_CHANGES_TO_MERGE);
    log_info(COMP_MERGE, "merge succeeded");
    return gk_session_success(session);
}
