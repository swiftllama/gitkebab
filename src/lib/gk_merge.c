
#include <string.h>
#include "git2.h"

#include "gk_merge.h"
#include "gk_types.h"
#include "gk_logging.h"
#include "gk_repository.h"
#include "gk_conflicts.h"
#include "gk_lg2_private.h"

int gk_repository_analyze_merge_into_head(gk_repository *repository, const char* from_ref_name, int *out_analysis) {
    if (gk_repository_verify(repository, &COMP_MERGE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT, "analyze merge") != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (from_ref_name == NULL) {
        return gk_repository_failure(repository, &COMP_MERGE, -5, "Cannot analyze merge from NULL reference");
    }

    log_info(COMP_MERGE, "Initiating merge analysis for merging ref '%s' into HEAD in repository '%s'", from_ref_name, repository->repository_spec.local_path);
    git_merge_analysis_t analysis;
    git_merge_preference_t preference;

    int rc = git_merge_analysis(&analysis, &preference, repository->lg2_resources->repository, (const git_annotated_commit **)&repository->lg2_resources->annotated_fetch_head_commit, 1);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_repository_failure(repository, &COMP_MERGE, -5, "Cannot analyze merge from reference '%s' (%d): %s", from_ref_name, err->klass, err->message);
    }
    
    log_info(COMP_MERGE, "Merge analysis result is: %d", analysis);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_NORMAL: %d", (analysis & GIT_MERGE_ANALYSIS_NORMAL) != 0 ? GIT_MERGE_ANALYSIS_NORMAL : 0);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_UP_TO_DATE: %d", (analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) != 0 ? GIT_MERGE_ANALYSIS_UP_TO_DATE : 0);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_FASTFORWARD: %d", (analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0 ? GIT_MERGE_ANALYSIS_FASTFORWARD : 0);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_UNBORN: %d", (analysis & GIT_MERGE_ANALYSIS_UNBORN) != 0 ? GIT_MERGE_ANALYSIS_UNBORN : 0);


    if (analysis == GIT_MERGE_ANALYSIS_UP_TO_DATE) {
        gk_repository_state_unset(repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE);
    }
    else {
        gk_repository_state_set(repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE);
    }
    log_info(COMP_MERGE, "Set repository has changes to merge to %d", gk_repository_state_enabled(repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE));
    if (analysis == GIT_MERGE_ANALYSIS_UNBORN) {
        log_warn(COMP_MERGE, "Repositor merge analysis resulted UNBORN, this is unexpected");
    }

    if (out_analysis != NULL) {
        *out_analysis = analysis;
    }
    
    return gk_repository_success(repository);
}

static int create_merge_commit(gk_repository *repository) {
    // Find parents
    if (gk_lg2_parents_lookup(repository, "create merge commit") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    if (gk_lg2_signature_create(repository, &COMP_MERGE, "create merge commit") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    char commit_message[128];
    snprintf(commit_message, 128, "Merge %s into %s", repository->repository_spec.remote_ref_name, repository->repository.main_branch_name);
    git_oid commit_oid;
    int rc = git_commit_create(&commit_oid,
                           repository->lg2_resources->repository, git_reference_name(repository->lg2_resources->repository_head_ref),
                           repository->lg2_resources->signature, repository->lg2_resources->signature,
                           NULL, commit_message,
                           repository->lg2_resources->tree,
                           2, (const git_commit **)repository->lg2_resources->merge_parents);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_repository_failure(repository, &COMP_MERGE, -5, "Cannot create merge commit, error creating commit (%d): %s", err->klass, err->message);
    }

    log_info(COMP_MERGE, "Created merge commit '%s'", git_oid_tostr_s(&commit_oid));
    git_repository_state_cleanup(repository->lg2_resources->repository);

    return GK_SUCCESS;    
}


static int merge_in_memory(gk_repository *repository) {
    git_merge_options merge_options = GIT_MERGE_OPTIONS_INIT;

    merge_options.flags = 0;
    merge_options.file_flags = GIT_MERGE_FILE_STYLE_DIFF3;

    gk_lg2_merge_index_free(repository);
    int rc = git_merge_commits(&repository->lg2_resources->merge_index, repository->lg2_resources->repository, repository->lg2_resources->repository_head_commit, repository->lg2_resources->fetch_head_commit, &merge_options);
    if (rc != 0) {
        gk_lg2_merge_index_free(repository);
        const git_error *err = git_error_last();
        gk_repository_failure(repository, &COMP_MERGE, -5, "Error performing in-memory merge (%d): %s", err->klass, err->message);
        return GK_FAILURE;
    }

    if (git_index_has_conflicts(repository->lg2_resources->merge_index) == 0) {
        if (gk_lg2_promote_merge_index(repository, "merge in memory") != GK_SUCCESS) {
            return GK_FAILURE;
        }
        rc = create_merge_commit(repository);
        gk_lg2_merge_index_free(repository);
        if (rc != GK_SUCCESS) {
            return GK_FAILURE;
        }
        gk_repository_state_unset(repository, GK_REPOSITORY_STATE_HAS_CONFLICTS);
        gk_repository_state_unset(repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE);
    }
    else { // has conflicts
        log_error(COMP_MERGE, "detected conflicts after merge");
        if (gk_repository_merge_conflicts_query(repository, "merge in memory") != GK_SUCCESS) {
            return GK_FAILURE;
        }
        gk_repository_state_set(repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE);
        gk_repository_state_set(repository, GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING);
    }

    return GK_SUCCESS;
}

int gk_repository_merge_conflicts_query(gk_repository *repository, const char *purpose) {
    if (gk_repository_verify(repository, &COMP_MERGE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT | GK_REPOSITORY_VERIFY_MERGE_IN_PROGRESS, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (gk_lg2_iterate_conflicts(repository, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (repository->conflict_summary.num_conflicts > 0) {
        gk_repository_state_set(repository, GK_REPOSITORY_STATE_HAS_CONFLICTS);
    }
    else {
        gk_repository_state_unset(repository, GK_REPOSITORY_STATE_HAS_CONFLICTS);
    }
    return GK_SUCCESS;
}

/*
static int merge_normal(gk_repository *repository) {    
    git_merge_options merge_options = GIT_MERGE_OPTIONS_INIT;
    git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;

    merge_options.flags = 0;
    merge_options.file_flags = GIT_MERGE_FILE_STYLE_DIFF3;

    gk_authed authed;
    gk_authed_init(&authed, repository, NULL);
    
    checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE | GIT_CHECKOUT_ALLOW_CONFLICTS;
    checkout_options.progress_cb = gk_repository_checkout_progress_callback;
    checkout_options.progress_payload = &authed;
    
    int rc = git_merge(repository->lg2_resources->repository, (const git_annotated_commit **)&repository->lg2_resources->annotated_fetch_head_commit, 1, &merge_options, &checkout_options);
    if (rc != 0) {
        const git_error *err = git_error_last();
        gk_repository_failure(repository, &COMP_MERGE, -5, "Error performing normal merge (%d): %s", err->klass, err->message);
        return GK_FAILURE;
    }

    rc = gk_lg2_index_load(repository, "perform normal merge");
    if (rc != GK_SUCCESS) {
        return GK_FAILURE;
    }

    if (git_index_has_conflicts(repository->lg2_resources->index) == 1) {
        log_info(COMP_MERGE, "Encountered conflicts after normal merge, setting state to GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING");
        gk_repository_state_set(repository, GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING | GK_REPOSITORY_STATE_HAS_CONFLICTS);
    }
    else {
        if (gk_lg2_index_write_tree(repository, repository->lg2_resources->index, "peroform normal merge") != GK_SUCCESS) {
            return GK_FAILURE;
        }
        rc = create_merge_commit(repository);
        if (rc != GK_SUCCESS) {
            return GK_FAILURE;
        }
    }

    return gk_repository_success(repository);
    }*/

static int merge_fast_forward(gk_repository *repository, const char *from_ref_name) {    
    log_info(COMP_MERGE, "Fast-forwardig merge from current HEAD at '%s' to %s at '%s'", repository->lg2_resources->repository_head_oid_id , from_ref_name, repository->lg2_resources->fetch_head_oid_id);
    
    gk_authed authed;
    gk_authed_init(&authed, repository, NULL);
    
    git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_options.progress_cb = gk_repository_checkout_progress_callback;
    checkout_options.progress_payload = &authed;

    log_debug(COMP_MERGE, "Checked out tree at rev '%s' into working directory", repository->lg2_resources->fetch_head_oid_id);
    int rc = git_checkout_tree(repository->lg2_resources->repository, repository->lg2_resources->fetch_head_object, &checkout_options);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_repository_failure(repository, &COMP_MERGE, -6, "Error preforming fast-forward merge, checkout failed (%d): %s", err->klass, err->message);
    }

    git_reference *new_head_ref = NULL;
    log_debug(COMP_MERGE, "Advancing HEAD to rev '%s'", repository->lg2_resources->fetch_head_oid_id);
    /* Move the target reference to the target OID */
    rc = git_reference_set_target(&new_head_ref, repository->lg2_resources->repository_head_ref, repository->lg2_resources->fetch_head_oid, NULL);
    if (rc != 0) {
        const git_error *err = git_error_last();
        git_reference_free(new_head_ref);
        return gk_repository_failure(repository, &COMP_MERGE, -6, "Error preforming fast-forward merge, failed to advance head to oid '%s' (%d): %s", repository->lg2_resources->fetch_head_oid_id, err->klass, err->message);
    }

    gk_repository_state_unset(repository, GK_REPOSITORY_STATE_HAS_CONFLICTS);
    gk_repository_state_unset(repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE);
    
    return gk_repository_success(repository);
}
    
int gk_repository_merge_into_head(gk_repository *repository) {
    const char* from_ref_name = repository->repository_spec.remote_ref_name;
    int merge_analysis = 0;
    log_info(COMP_MERGE, "merging '%s' into HEAD", from_ref_name);
    gk_repository_state_set(repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);

    if (gk_lg2_load_references(repository) != GK_SUCCESS) {
        gk_lg2_free_all_but_repository(repository);
        return GK_FAILURE;
    }

    if (gk_repository_state_enabled(repository, GK_REPOSITORY_STATE_MERGE_PENDING_ON_DISK)) {
        return gk_repository_failure(repository, &COMP_MERGE, -5, "UNIMPLEMENTED: merge pending on disk");
    }
    
    int rc = gk_repository_analyze_merge_into_head(repository, from_ref_name, &merge_analysis);
    if (rc == GK_FAILURE) {
        gk_repository_state_unset(repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);
        gk_lg2_free_all_but_repository(repository);
        return GK_FAILURE;
    }

    if ((merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0) {
        log_info(COMP_MERGE, "will attempt a fast-forward merge");
        rc = merge_fast_forward(repository, from_ref_name);
        if (rc == GK_FAILURE) {
            log_info(COMP_MERGE, "Fast-forward merge failed: %s (%d)", gk_result_message(repository->last_result), gk_result_code(repository->last_result));
            gk_repository_state_unset(repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);
            gk_lg2_free_all_but_repository(repository);
            return GK_FAILURE;
        }
    }
    else if ((merge_analysis & GIT_MERGE_ANALYSIS_NORMAL) != 0) {
        log_info(COMP_MERGE, "will attempt an in-memory merge");
        rc = merge_in_memory(repository);
        if (rc == GK_FAILURE) {
            log_info(COMP_MERGE, "in-memory merge failed: %s (%d)", gk_result_message(repository->last_result), gk_result_code(repository->last_result));
            gk_repository_state_unset(repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);
            gk_lg2_free_all_but_repository(repository);
            return GK_FAILURE;
        }
    }
    else if ((merge_analysis & GIT_MERGE_ANALYSIS_UNBORN) != 0) {
        gk_repository_state_unset(repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);
        gk_lg2_free_all_but_repository(repository);
        return gk_repository_failure(repository, &COMP_MERGE, -4, "Error merging changes from server: head points to an unknonw commit id");
    }
    else if ((merge_analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) != 0) {
        log_info(COMP_MERGE, "HEAD is up to date with '%s', no merge is necessary", from_ref_name);
        // nothing to do
    }
    else {
        log_warn(COMP_MERGE, "unknown merge analysis state %d while merging, no merge will be performed", merge_analysis);
    }

    gk_repository_state_unset(repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);

    if (gk_repository_state_enabled(repository, GK_REPOSITORY_STATE_HAS_CONFLICTS)) {
        log_info(COMP_MERGE, "merge attempt ended with conflicts");
    }
    else if (gk_repository_state_enabled(repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE)) {
        log_info(COMP_MERGE, "merge attempt ended without conflicts but merge still pending");
    }
    else {
        log_info(COMP_MERGE, "merge attempt ended, all changes merged");
    }

    gk_lg2_free_all_but_repository(repository);
    return gk_repository_success(repository);
}

int gk_repository_merge_into_head_finalize(gk_repository *repository) {
    const char *purpose = "finalize merge into head";
    if (gk_repository_verify(repository, &COMP_MERGE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT | GK_REPOSITORY_VERIFY_MERGE_IN_PROGRESS, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    if (git_index_has_conflicts(repository->lg2_resources->merge_index) == 1) {
        if (gk_repository_merge_conflicts_query(repository, purpose) != GK_SUCCESS) {
            return GK_FAILURE;
        }
        return gk_repository_failure(repository, &COMP_CONFLICTS, -1, "Cannot %s, repository still has %d conflicts", purpose, repository->conflict_summary.num_conflicts);
    }

    if (gk_lg2_load_references(repository) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    if (strcmp(repository->conflict_summary.repository_head_oid_id, repository->lg2_resources->repository_head_oid_id) != 0) {
        log_warn(COMP_MERGE, "Cannot %s, conflict summary was calculated at repository head [%s] but the repository head is now [%s]", purpose, repository->conflict_summary.repository_head_oid_id, repository->lg2_resources->repository_head_oid_id);
        if (gk_repository_merge_abort(repository) != GK_SUCCESS) {
            return gk_repository_failure(repository, &COMP_MERGE, -13, "Cannot %s, error aborting repository due to repository head mismatch: %s", purpose, gk_result_message(repository->last_result));
        }
        return gk_repository_failure(repository, &COMP_MERGE, -14, "Cannot %s, conflict summary was calculated with repository head [%s] but the repository head is now [%s]", purpose, repository->conflict_summary.repository_head_oid_id, repository->lg2_resources->repository_head_oid_id);
    }

    if (strcmp(repository->conflict_summary.fetch_head_oid_id, repository->lg2_resources->fetch_head_oid_id) != 0) {
        if (gk_repository_merge_abort(repository) != GK_SUCCESS) {
            return gk_repository_failure(repository, &COMP_MERGE, -13, "Cannot %s, error aborting repository due to fetch head mismatch: %s", purpose, gk_result_message(repository->last_result));
        }
        return gk_repository_failure(repository, &COMP_MERGE, -14, "Cannot %s, conflict summary was calculated with fetch head [%s] but the fetch head is now [%s]", purpose, repository->conflict_summary.fetch_head_oid_id, repository->lg2_resources->fetch_head_oid_id);
    }

    if (gk_lg2_promote_merge_index(repository, "finalize merge into head") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    if (create_merge_commit(repository) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_repository_state_unset(repository, GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING);

    log_info(COMP_MERGE, "Successfully finalized merge into head");
    return GK_SUCCESS;
}

    int gk_repository_merge_abort(gk_repository *repository) {
    const char *purpose = "abort merge into head";
    if (gk_repository_verify(repository, &COMP_MERGE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT | GK_REPOSITORY_VERIFY_MERGE_IN_PROGRESS, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_conflicts_free(repository);
    gk_lg2_merge_index_free(repository);
    gk_repository_state_unset(repository, GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING);
    log_info(COMP_MERGE, "Aborting merge into head");
    return GK_SUCCESS;
}
