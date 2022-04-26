
#include <string.h>

#include "git2.h"

#include "gk_merge.h"
#include "gk_types.h"
#include "gk_logging.h"
#include "gk_repository.h"
#include "gk_conflicts.h"
#include "gk_lg2_private.h"
#include "gk_session.h"

static int unset_merge_in_progress_and_trigger_state_callback(gk_session *session, const char *purpose) {
    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);
    if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }
    return GK_SUCCESS;
}


int gk_analyze_merge_into_head(gk_session *session, const char* from_ref_name, int *out_analysis) {
    const char *purpose = "analyze merge into HEAD";
    if (gk_session_context_push(session, purpose, &COMP_MERGE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (from_ref_name == NULL) {
        return gk_session_failure_ex(session, purpose, GK_ERR, "remote reference is NULL");
    }

    log_info(COMP_MERGE, "Initiating merge analysis for merging ref [%s] into HEAD in repository [%s]", from_ref_name, session->repository->spec.local_path);
    git_merge_analysis_t analysis;
    git_merge_preference_t preference;

    if (session->repository->lg2_resources->annotated_fetch_head_commit == NULL) {
        log_info(COMP_MERGE, "fetch head is NULL, interpreting as remote being empty, no merge needs to be performed");
        analysis = GIT_MERGE_ANALYSIS_UP_TO_DATE;
    }
    else {
        if (git_merge_analysis(&analysis, &preference, session->repository->lg2_resources->repository, (const git_annotated_commit **)&session->repository->lg2_resources->annotated_fetch_head_commit, 1) != 0) {
            return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "Cannot analyze merge from reference [%s]", from_ref_name);
        }
    }
    
    log_info(COMP_MERGE, "Merge analysis result is: %d", analysis);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_NORMAL: %d", (analysis & GIT_MERGE_ANALYSIS_NORMAL) != 0 ? GIT_MERGE_ANALYSIS_NORMAL : 0);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_UP_TO_DATE: %d", (analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) != 0 ? GIT_MERGE_ANALYSIS_UP_TO_DATE : 0);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_FASTFORWARD: %d", (analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0 ? GIT_MERGE_ANALYSIS_FASTFORWARD : 0);
    log_info(COMP_MERGE, "MERGE_ANALYSIS_UNBORN: %d", (analysis & GIT_MERGE_ANALYSIS_UNBORN) != 0 ? GIT_MERGE_ANALYSIS_UNBORN : 0);


    if (analysis == GIT_MERGE_ANALYSIS_UP_TO_DATE) {
        gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE);
    }
    else {
        gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE);
    }
    log_info(COMP_MERGE, "Set repository has changes to merge to %d", gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE));
    if (analysis == GIT_MERGE_ANALYSIS_UNBORN) {
        log_warn(COMP_MERGE, "Repositor merge analysis resulted UNBORN, this is unexpected");
    }

    if (out_analysis != NULL) {
        *out_analysis = analysis;
    }
    
    return gk_session_success(session, purpose);
}

static int create_merge_commit(gk_session *session) {
    const char *purpose = "create merge commit";
    if (gk_session_context_push(session, purpose, &COMP_MERGE, GK_REPOSITORY_VERIFY_DEFAULT) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    // Find parents
    if (gk_lg2_parents_lookup(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }

    if (gk_lg2_signature_create(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }

    char commit_message[128];
    snprintf(commit_message, 128, "Merge %s into %s", session->repository->spec.remote_ref_name, session->repository->spec.main_branch_name);
    git_oid commit_oid;
    int rc = git_commit_create(&commit_oid,
                           lg2_resources->repository, git_reference_name(lg2_resources->repository_head_ref),
                           lg2_resources->signature, lg2_resources->signature,
                           NULL, commit_message,
                           lg2_resources->tree,
                           2, (const git_commit **)lg2_resources->merge_parents);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure_ex(session, purpose, GK_ERR, "Cannot create merge commit, error creating commit (%d): %s", err->klass, err->message);
    }

    log_info(COMP_MERGE, "Created merge commit '%s'", git_oid_tostr_s(&commit_oid));
    git_repository_state_cleanup(lg2_resources->repository);

    return gk_session_success(session, purpose);    
}


static int merge_in_memory(gk_session *session) {
    const char *purpose = "merge in memory";
    if (gk_session_context_push(session, purpose, &COMP_MERGE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    git_merge_options merge_options = GIT_MERGE_OPTIONS_INIT;

    merge_options.flags = 0;
    merge_options.file_flags = GIT_MERGE_FILE_STYLE_DIFF3;

    gk_lg2_merge_index_free(session->repository);
    int rc = git_merge_commits(&lg2_resources->merge_index, lg2_resources->repository, lg2_resources->repository_head_commit, lg2_resources->fetch_head_commit, &merge_options);
    if (rc != 0) {
        gk_lg2_merge_index_free(session->repository);
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to merge in-memory");
    }

    if (git_index_has_conflicts(lg2_resources->merge_index) == 0) {
        if (gk_lg2_promote_merge_index(session) != GK_SUCCESS) {
            return gk_session_failure(session, purpose);
        }
        rc = create_merge_commit(session);
        gk_lg2_merge_index_free(session->repository);
        if (rc != GK_SUCCESS) {
            return gk_session_failure(session, purpose);
        }
        gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_HAS_CONFLICTS);
        gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE);
    }
    else { // has conflicts
        log_error(COMP_MERGE, "detected conflicts after merge");
        if (gk_merge_conflicts_query(session) != GK_SUCCESS) {
            return gk_session_failure(session, purpose);
        }
        gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE);
        gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING);
    }

    return gk_session_success(session, purpose);
}

int gk_merge_conflicts_query(gk_session *session) {
    const char *purpose = "query merge conflicts";
    if (gk_session_context_push(session, purpose, &COMP_MERGE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT | GK_REPOSITORY_VERIFY_MERGE_INDEX_LOADED) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    if (gk_lg2_iterate_conflicts(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }
    if (session->repository->conflict_summary.num_conflicts > 0) {
        gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_HAS_CONFLICTS);
    }
    else {
        gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_HAS_CONFLICTS);
    }
    if (session->callbacks.merge_conflict_query_callback != NULL) {
        session->callbacks.merge_conflict_query_callback(session->id_ptr, session->repository);
    }
    if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }
    return gk_session_success(session, purpose);
}

/*
static int merge_normal(gk_session *session) {    
    git_merge_options merge_options = GIT_MERGE_OPTIONS_INIT;
    git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;

    merge_options.flags = 0;
    merge_options.file_flags = GIT_MERGE_FILE_STYLE_DIFF3;

    gk_authed authed;
    gk_authed_init(&authed, repository, NULL);
    
    checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE | GIT_CHECKOUT_ALLOW_CONFLICTS;
    checkout_options.progress_cb = gk_repository_checkout_progress_callback;
    checkout_options.progress_payload = session;
    
    int rc = git_merge(repository->lg2_resources->repository, (const git_annotated_commit **)&repository->lg2_resources->annotated_fetch_head_commit, 1, &merge_options, &checkout_options);
    if (rc != 0) {
        const git_error *err = git_error_last();
        gk_session_failure_ex(session, purpose, GK_ERR, "Error performing normal merge (%d): %s", err->klass, err->message);
        return GK_FAILURE;
    }

    rc = gk_lg2_index_load(session);
    if (rc != GK_SUCCESS) {
        return GK_FAILURE;
    }

    if (git_index_has_conflicts(repository->lg2_resources->index) == 1) {
        log_info(COMP_MERGE, "Encountered conflicts after normal merge, setting state to GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING");
        gk_repository_state_set(repository, GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING | GK_REPOSITORY_STATE_HAS_CONFLICTS);
    }
    else {
        if (gk_lg2_index_write_tree(session, session->repository->lg2_resources->index) != GK_SUCCESS) {
            return GK_FAILURE;
        }
        rc = create_merge_commit(repository);
        if (rc != GK_SUCCESS) {
            return GK_FAILURE;
        }
    }

    return gk_session_success(session);
    }*/

static int merge_fast_forward(gk_session *session, const char *from_ref_name) {
    const char *purpose = "analyze merge into HEAD";
    if (gk_session_context_push(session, purpose, &COMP_MERGE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT | GK_REPOSITORY_VERIFY_MERGE_IN_PROGRESS) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    log_info(COMP_MERGE, "Fast-forwardig merge from current HEAD at [%s] to %s at [%s]", lg2_resources->repository_head_oid_id , from_ref_name, lg2_resources->fetch_head_oid_id);
    
    git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_options.progress_cb = gk_session_checkout_progress_callback;
    checkout_options.progress_payload = session;

    if (lg2_resources->fetch_head_object != NULL) {
        // If if the fetch head is NULL, remote repo is empty, so
        // there's nothing to actually merge. For this assumption to
        // be valid, gk_lg2_load_references() must have been called at
        // some recent point, to be sure that it tried to load the
        // remote fetch head
        log_debug(COMP_MERGE, "Checked out tree at rev [%s] for reference [%s] into working directory", lg2_resources->fetch_head_oid_id, from_ref_name);
        if (git_checkout_tree(lg2_resources->repository, lg2_resources->fetch_head_object, &checkout_options) != 0) {
            return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to checkout out tree at rev [%s]", lg2_resources->fetch_head_oid_id);
        }
        
        git_reference *new_head_ref = NULL;
        log_debug(COMP_MERGE, "Advancing HEAD to rev '%s'", lg2_resources->fetch_head_oid_id);
        if (lg2_resources->repository_head_ref == NULL) {
            // Repository is empty, create head reference
            
            char ref_name[128];
            snprintf(ref_name, 128, "refs/heads/%s", session->repository->spec.main_branch_name);
            if (git_commit_lookup(&lg2_resources->repository_head_commit, lg2_resources->repository, lg2_resources->fetch_head_oid) != 0) {
                return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to look up fetch head oid [%s] while creating main branch for incoming commits from remote main branch", lg2_resources->fetch_head_oid_id);
            }
            if (git_reference_create(&lg2_resources->repository_head_ref, lg2_resources->repository, ref_name, lg2_resources->fetch_head_oid, 0, "create main branch for incoming commits on remote main branch") != 0) {
                return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to create head reference for branch [%s] while creating main branch for incoming commits from remote main branch", session->repository->spec.main_branch_name);
            }
        }
        else {
            // Repository has other commits, move the current head to point to the newly fetched commit
            if (git_reference_set_target(&new_head_ref, lg2_resources->repository_head_ref, lg2_resources->fetch_head_oid, NULL) != 0) {
                git_reference_free(new_head_ref);
                return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to advance head to oid [%s]", lg2_resources->fetch_head_oid_id);
            }
        }
    }

    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_HAS_CONFLICTS);
    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE);
    
    return gk_session_success(session, purpose);
}
    
int gk_merge_into_head(gk_session *session) {
    const char *purpose = "merge into HEAD";
    if (gk_session_context_push(session, purpose, &COMP_MERGE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    const char* from_ref_name = session->repository->spec.remote_ref_name;

    int merge_analysis = 0;
    log_info(COMP_MERGE, "merging [%s] into HEAD", from_ref_name);
    gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);
    if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }

    if (gk_lg2_load_references(session) != GK_SUCCESS) {
        gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);
        if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
            return gk_session_failure(session, purpose);
        }
        gk_lg2_free_all_but_repository(session->repository);
        return gk_session_failure(session, purpose);
    }

    if (gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_MERGE_PENDING_ON_DISK)) {
        gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);
        if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
            return gk_session_failure(session, purpose);
        }
        return gk_session_failure_ex(session, purpose, GK_ERR, "UNIMPLEMENTED: merge pending on disk");
    }
    
    if (gk_analyze_merge_into_head(session, from_ref_name, &merge_analysis) != GK_SUCCESS) {
        gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);
        if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
            return gk_session_failure(session, purpose);
        }
        gk_lg2_free_all_but_repository(session->repository);
        return gk_session_failure(session, purpose);
    }

    if ((merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0) {
        log_info(COMP_MERGE, "will attempt a fast-forward merge");
        if (merge_fast_forward(session, from_ref_name) != GK_SUCCESS) {
            gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);
            if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
                return gk_session_failure(session, purpose);
            }
            gk_lg2_free_all_but_repository(session->repository);
            return gk_session_failure(session, purpose);
        }
    }
    else if ((merge_analysis & GIT_MERGE_ANALYSIS_NORMAL) != 0) {
        log_info(COMP_MERGE, "will attempt an in-memory merge");
        if (merge_in_memory(session) != GK_SUCCESS) {
            gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);
            if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
                return gk_session_failure(session, purpose);
            }
            gk_lg2_free_all_but_repository(session->repository);
            return gk_session_failure(session, purpose);
        }
    }
    else if ((merge_analysis & GIT_MERGE_ANALYSIS_UNBORN) != 0) {
        gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);
        if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
            return gk_session_failure(session, purpose);
        }
        gk_lg2_free_all_but_repository(session->repository);
        return gk_session_failure_ex(session, purpose, GK_ERR, "head points to an unknonw commit id");
    }
    else if ((merge_analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) != 0) {
        log_info(COMP_MERGE, "HEAD is up to date with [%s], no merge is necessary", from_ref_name);
        // nothing to do
    }
    else {
        log_warn(COMP_MERGE, "unknown merge analysis state [%d] while merging, no merge will be performed", merge_analysis);
    }

    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);

    if (gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_HAS_CONFLICTS)) {
        log_info(COMP_MERGE, "merge attempt ended with conflicts");
    }
    else if (gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE)) {
        log_info(COMP_MERGE, "merge attempt ended without conflicts but merge still pending");
    }
    else {
        log_info(COMP_MERGE, "merge attempt ended, all changes merged");
    }

    if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }

    gk_lg2_free_all_but_repository(session->repository);
    return gk_session_success(session, purpose);
}

int gk_merge_into_head_finalize(gk_session *session) {
    const char *purpose = "finalize merge into head";
    if (gk_session_context_push(session, purpose, &COMP_MERGE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT | GK_REPOSITORY_VERIFY_MERGE_INDEX_LOADED) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS);
    if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }
    
    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    if (git_index_has_conflicts(lg2_resources->merge_index) == 1) {
        if (gk_merge_conflicts_query(session) != GK_SUCCESS) {
            if (unset_merge_in_progress_and_trigger_state_callback(session, purpose) != 0) { return GK_ERR; }
            return gk_session_failure_ex(session, purpose, GK_ERR, "failed to query merge after conflicts detected");
        }
        if (unset_merge_in_progress_and_trigger_state_callback(session, purpose) != 0) { return GK_ERR; }
        return gk_session_failure_ex(session, purpose, GK_ERR_MERGE_HAS_CONFLICTS, "repository still has [%d] conflicts", session->repository->conflict_summary.num_conflicts);
    }

    if (gk_lg2_load_references(session) != GK_SUCCESS) {
        if (unset_merge_in_progress_and_trigger_state_callback(session, purpose) != 0) { return GK_ERR; }
        return gk_session_failure(session, purpose);
    }

    if (strcmp(session->repository->conflict_summary.repository_head_oid_id, lg2_resources->repository_head_oid_id) != 0) {
        log_warn(COMP_MERGE, "Cannot %s, conflict summary was calculated at repository head [%s] but the repository head is now [%s]", purpose, session->repository->conflict_summary.repository_head_oid_id, lg2_resources->repository_head_oid_id);
        if (gk_merge_abort(session) != GK_SUCCESS) {
            if (unset_merge_in_progress_and_trigger_state_callback(session, purpose) != 0) { return GK_ERR; }
            return gk_session_failure_ex(session, purpose, GK_ERR, "repository head mismatch");
        }
        if (unset_merge_in_progress_and_trigger_state_callback(session, purpose) != 0) { return GK_ERR; }
        return gk_session_failure_ex(session, purpose, GK_ERR, "conflict summary was calculated with repository head [%s] but the repository head is now [%s]", session->repository->conflict_summary.repository_head_oid_id, lg2_resources->repository_head_oid_id);
    }

    if (strcmp(session->repository->conflict_summary.fetch_head_oid_id, lg2_resources->fetch_head_oid_id) != 0) {
        if (gk_merge_abort(session) != GK_SUCCESS) {
            if (unset_merge_in_progress_and_trigger_state_callback(session, purpose) != 0) { return GK_ERR; }
            return gk_session_failure_ex(session, purpose, GK_ERR, "fetch head mismatch");
        }
        if (unset_merge_in_progress_and_trigger_state_callback(session, purpose) != 0) { return GK_ERR; }
        return gk_session_failure_ex(session, purpose, GK_ERR, "conflict summary was calculated with fetch head [%s] but the fetch head is now [%s]", session->repository->conflict_summary.fetch_head_oid_id, lg2_resources->fetch_head_oid_id);
    }

    if (gk_lg2_promote_merge_index(session) != GK_SUCCESS) {
        if (unset_merge_in_progress_and_trigger_state_callback(session, purpose) != 0) { return GK_ERR; }
        return gk_session_failure(session, purpose);
    }

    if (create_merge_commit(session) != GK_SUCCESS) {
        if (unset_merge_in_progress_and_trigger_state_callback(session, purpose) != 0) { return GK_ERR; }
        return gk_session_failure(session, purpose);
    }

    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE);
    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING);
    
    if (unset_merge_in_progress_and_trigger_state_callback(session, purpose) != 0) {
        return GK_ERR;
    }

    log_info(COMP_MERGE, "Successfully finalized merge into head");
    return gk_session_success(session, purpose);
}

int gk_merge_abort(gk_session *session) {
    const char *purpose = "abort merge into HEAD";
    if (gk_session_context_push(session, purpose, &COMP_MERGE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT | GK_REPOSITORY_VERIFY_MERGE_INDEX_LOADED) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_conflicts_free(session->repository);
    gk_lg2_merge_index_free(session->repository);
    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING);
    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_HAS_CONFLICTS);
    if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }
    if (session->callbacks.merge_conflict_query_callback != NULL) {
        session->callbacks.merge_conflict_query_callback(session->id_ptr, session->repository);
    }
    log_info(COMP_MERGE, "Aborted merge into head");
    return gk_session_success(session, purpose);
}

const char *gk_merge_conflict_entry_ancestor_oid_id(gk_merge_conflict_entry *entry) {
    if (entry == NULL) {
        log_error(COMP_MERGE, "Cannot return ancestor commit id for NULL entry, returning an empty string");
        return "";
    }
    return entry->ancestor_oid_id;
}

const char *gk_merge_conflict_entry_ours_oid_id(gk_merge_conflict_entry *entry) {
    if (entry == NULL) {
        log_error(COMP_MERGE, "Cannot return ours commit id for NULL entry, returning an empty string");
        return "";
    }
    return entry->ours_oid_id;
}

const char *gk_merge_conflict_entry_theirs_oid_id(gk_merge_conflict_entry *entry) {
    if (entry == NULL) {
        log_error(COMP_MERGE, "Cannot return theirs commit id for NULL entry, returning an empty string");
        return "";
    }
    return entry->theirs_oid_id;
}

const char *gk_merge_conflict_summary_fetch_head_oid_id(gk_repository *repository) {
    if (repository == NULL) {
        log_error(COMP_MERGE, "Cannot return fetch head oid id for NULL repository, returning an empty string");
        return "";
    }
    return repository->conflict_summary.fetch_head_oid_id;
}

const char *gk_merge_conflict_summary_repository_head_oid_id(gk_repository *repository) {
    if (repository == NULL) {
        log_error(COMP_MERGE, "Cannot return repository head oid id for NULL repository, returning an empty string");
        return "";
    }
    return repository->conflict_summary.repository_head_oid_id;
}

