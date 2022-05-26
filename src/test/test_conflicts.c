
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"
#include "git2.h"
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"
#include "gk_test_env_utils.h"

const char *error_listing = "merge";

static int test_staging_setup(void **state) {
    gk_init(NULL, LOG_DEBUG);

    prepare_error_listing(error_listing);
    //gk_libgit2_set_log_level(LOG_DEBUG);
    return gk_test_environment_setup(state);
}

static int test_staging_teardown(void **state) {
    return gk_test_environment_teardown(state);
}

static int test_staging_clean_repo_setup(void **state) {
    (void) state;
    gk_test_delete_simplerepo1();
    gk_test_delete_simplerepo1A();
    gk_test_delete_simplerepo1B();
    gk_test_delete_simplerepo1_dot_git();
    gk_test_copy_source_repo_simplerepo1_dot_git();
    return 0;
}

static void test_conflicts_various_types(void **state) {
    (void) state;

    // Setup repos A and B
    // A has committed and pushed various changes
    // B has committed and fetched, now has conflicts
    gk_session *session1 = NULL;
    gk_session *session2 = NULL;

    gk_test_reset_state_change_record();
    gk_test_env_conflicting_repos_a_and_b_with_extended_conflicts(&session1, &session2, state);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_FETCH_IN_PROGRESS), 1);
    assert_int_equal(gk_test_state_count_disabled(GK_REPOSITORY_STATE_FETCH_IN_PROGRESS), 1);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE), 1);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_HAS_CONFLICTS), 0); // don't know tha we have conflicts until we try to merge
    
    // Attempt a merge in repo B
    gk_test_reset_state_change_record();
    gk_merge_into_head(session2);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_MERGE_IN_PROGRESS), 1);
    assert_int_equal(gk_test_state_count_disabled(GK_REPOSITORY_STATE_MERGE_IN_PROGRESS), 1);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_HAS_CONFLICTS), 1);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    // We should now have 6 conflicts of various types
    gk_merge_conflict_summary *summary = &session2->repository->conflict_summary;
    assert_int_equal(summary->num_conflicts, 6);
    assert_string_equal(summary->conflicts[0]->path, "file1");
    assert_int_equal(summary->conflicts[0]->conflict_type, GK_MERGE_CONFLICT_LOCAL_DELETE_REMOTE_EDIT);
    assert_string_equal(summary->conflicts[1]->path, "file2");
    assert_int_equal(summary->conflicts[1]->conflict_type, GK_MERGE_CONFLICT_LOCAL_EDIT_REMOTE_DELETE);
    assert_string_equal(summary->conflicts[2]->path, "file3");
    assert_int_equal(summary->conflicts[2]->conflict_type, GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_EDIT);
    assert_string_equal(summary->conflicts[3]->path, "file4");
    assert_int_equal(summary->conflicts[3]->conflict_type, GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_EDIT);
    assert_string_equal(summary->conflicts[4]->path, "file5");
    assert_int_equal(summary->conflicts[4]->conflict_type, GK_MERGE_CONFLICT_LOCAL_DELETE_REMOTE_EDIT);
    assert_string_equal(summary->conflicts[5]->path, "file6");
    assert_int_equal(summary->conflicts[5]->conflict_type, GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_CREATE);
    
    gk_session_free(session1);
    gk_session_free(session2);
    }

static void test_conflicts_local_delete_remote_edit_file1(void **state) {
    (void) state;

    // Setup repos A and B
    // A has committed and pushed various changes
    // B has committed and fetched, now has conflicts
    gk_session *session1 = NULL;
    gk_session *session2 = NULL;
    gk_test_env_conflicting_repos_a_and_b_with_extended_conflicts(&session1, &session2, state);
    
    // Attempt a merge in repo B
    gk_merge_into_head(session2);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    // file1 should have a local-delete-remote-edit type conflict
    gk_merge_conflict_summary *summary = &session2->repository->conflict_summary;
    assert_int_equal(summary->num_conflicts, 6);
    assert_string_equal(summary->conflicts[0]->path, "file1");
    assert_int_equal(summary->conflicts[0]->conflict_type, GK_MERGE_CONFLICT_LOCAL_DELETE_REMOTE_EDIT);

    // write "theirs" version onto disk at new location
    int relativize_path = 1;
    gk_blob_write_contents(session2, summary->conflicts[0]->theirs_oid_id, "new_file1", relativize_path);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    // Verify that the file we "preserved" is the same as our edit
    assert_int_equal(diff("test-staging/simple-repo1-A/file1", "test-staging/simple-repo1-B/new_file1"), 0);
        
    gk_conflict_resolve_accept_remote_delete(session2, "file1");
    assert_int_equal(gk_session_last_result_code(session2), 0);

    gk_merge_conflicts_query(session2);
    assert_int_equal(gk_session_last_result_code(session2), 0);
    
    assert_int_equal(summary->num_conflicts, 5);
    assert_string_equal(summary->conflicts[0]->path, "file2");


    gk_session_free(session1);
    gk_session_free(session2);
}

static void test_conflicts_local_edit_remote_delete_file2(void **state) {
    (void) state;

    // Setup repos A and B
    // A has committed and pushed various changes
    // B has committed and fetched, now has conflicts
    gk_session *session1 = NULL;
    gk_session *session2 = NULL;
    gk_test_env_conflicting_repos_a_and_b_with_extended_conflicts(&session1, &session2, state);
    
    // Attempt a merge in repo B
    gk_merge_into_head(session2);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    // file2 should have a local-edit-remote-delete type conflict
    gk_merge_conflict_summary *summary = &session2->repository->conflict_summary;
    assert_int_equal(summary->num_conflicts, 6);
    assert_string_equal(summary->conflicts[1]->path, "file2");
    assert_int_equal(summary->conflicts[1]->conflict_type, GK_MERGE_CONFLICT_LOCAL_EDIT_REMOTE_DELETE);
    
    int relativize_path = 1;
    gk_blob_write_contents(session2, summary->conflicts[1]->ours_oid_id, "new_file2", relativize_path);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    // Verify that the file we "preserved" is the same as the incoming edit
    assert_int_equal(diff("test-staging/simple-repo1-B/file2", "test-staging/simple-repo1-B/new_file2"), 0);
        
    gk_conflict_resolve_accept_local_delete(session2, "file2");
    assert_int_equal(gk_session_last_result_code(session2), 0);

    gk_merge_conflicts_query(session2);
    assert_int_equal(gk_session_last_result_code(session2), 0);
    
    assert_int_equal(summary->num_conflicts, 5);
    assert_string_equal(summary->conflicts[1]->path, "file3");


    gk_session_free(session1);
    gk_session_free(session2);
}


static void test_conflicts_incompatible_twosided_edit(void **state) {
    (void) state;

    // Setup repos A and B
    // A has committed and pushed various changes
    // B has committed and fetched, now has conflicts
    gk_session *session1 = NULL;
    gk_session *session2 = NULL;
    gk_test_env_conflicting_repos_a_and_b_with_extended_conflicts(&session1, &session2, state);
    
    // Attempt a merge in repo B
    gk_merge_into_head(session2);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    // file3 should have a twosided-incompatible-edit type conflict
    gk_merge_conflict_summary *summary = &session2->repository->conflict_summary;
    assert_int_equal(summary->num_conflicts, 6);
    assert_string_equal(summary->conflicts[2]->path, "file3");
    assert_int_equal(summary->conflicts[2]->conflict_type, GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_EDIT);

    // Similarity should be high, i.e. between 80 and 100 (same file's
    // been modified in only a few places, albeit incompatibly)
    int similarity = 0;
    gk_compare_blobs(session2, &similarity, summary->conflicts[2]->ours_oid_id, summary->conflicts[2]->theirs_oid_id);
    assert_int_equal(gk_session_last_result_code(session2), 0);
    assert_in_range(similarity, 80, 101);
    
    const char *merged_buffer = gk_conflict_merged_buffer_with_conflict_markers(session2, summary->conflicts[2]->ancestor_oid_id, summary->conflicts[2]->ours_oid_id, summary->conflicts[2]->theirs_oid_id, "file3");
    assert_non_null(merged_buffer);
    //log_info(COMP_TEST, "Merged buffer: -----------------\n%s\n-------------------\n\n", merged_buffer);
    gk_conflict_merged_buffer_free(merged_buffer);

    char new_buffer[6] = "hello";
    gk_conflict_resolve_from_buffer(session2, "file3", new_buffer, 6);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    gk_merge_conflicts_query(session2);
    assert_int_equal(gk_session_last_result_code(session2), 0);
    summary = &session2->repository->conflict_summary;
    assert_int_equal(summary->num_conflicts, 5);
    assert_string_equal(summary->conflicts[2]->path, "file4");
    
    gk_session_free(session1);
    gk_session_free(session2);
}

static void test_conflicts_incompatible_twosided_create(void **state) {
    (void) state;

    // Setup repos A and B
    // A has committed and pushed various changes
    // B has committed and fetched, now has conflicts
    gk_session *session1 = NULL;
    gk_session *session2 = NULL;
    gk_test_env_conflicting_repos_a_and_b_with_extended_conflicts(&session1, &session2, state);
    
    // Attempt a merge in repo B
    gk_merge_into_head(session2);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    // file6 should have a twosided-incompatible-create type conflict
    gk_merge_conflict_summary *summary = &session2->repository->conflict_summary;
    assert_int_equal(summary->num_conflicts, 6);
    assert_string_equal(summary->conflicts[5]->path, "file6");
    assert_int_equal(summary->conflicts[5]->conflict_type, GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_CREATE);

    // Similarity should be low, i.e. between 0 and 20 (these are
    // two completely different files)
    int similarity = 0;
    gk_compare_blobs(session2, &similarity, summary->conflicts[5]->ours_oid_id, summary->conflicts[5]->theirs_oid_id);
    assert_int_equal(gk_session_last_result_code(session2), 0);
    assert_in_range(similarity, 0, 20);

    // Resolve by accepting theirs
    gk_conflict_resolve_accept_existing(session2, "file6", GK_CONFLICT_RESOLUTION_THEIRS);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    gk_merge_conflicts_query(session2);
    assert_int_equal(gk_session_last_result_code(session2), 0);
    summary = &session2->repository->conflict_summary;
    assert_int_equal(summary->num_conflicts, 5);
    assert_string_equal(summary->conflicts[4]->path, "file5");
                     
    gk_session_free(session1);
    gk_session_free(session2);
}

static void test_conflicts_partial_resolution_causes_failed_merge(void **state) {
    (void) state;

    // Setup repos A and B
    // A has committed and pushed various changes
    // B has committed and fetched, now has conflicts
    gk_session *session1 = NULL;
    gk_session *session2 = NULL;
    gk_test_env_conflicting_repos_a_and_b_with_extended_conflicts(&session1, &session2, state);
    
    // Attempt a merge in repo B
    gk_merge_into_head(session2);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    // We should now have 6 conflicts of various types
    gk_merge_conflict_summary *summary = &session2->repository->conflict_summary;
    assert_int_equal(summary->num_conflicts, 6);

    // Resolve two out of the six
    gk_conflict_resolve_accept_remote_delete(session2, "file1");
    assert_int_equal(gk_session_last_result_code(session2), 0);
    
    gk_conflict_resolve_accept_local_delete(session2, "file2");
    assert_int_equal(gk_session_last_result_code(session2), 0);

    // Query again, we should have four left
    gk_merge_conflicts_query(session2);
    assert_int_equal(gk_session_last_result_code(session2), 0);
    assert_int_equal(summary->num_conflicts, 4);

    // Try to finalize the merge, it should fail
    gk_merge_into_head_finalize(session2);
    append_to_error_listing(error_listing, "try to finalized merge into head while there are still conflicts", gk_result_code_as_string(gk_session_last_result_code(session2)), gk_session_last_result_message(session2));
    assert_int_equal(gk_session_last_result_code(session2), GK_ERR_MERGE_HAS_CONFLICTS);
    assert_int_equal(gk_repository_state_enabled(session2->repository, GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING), 1);
    assert_int_equal(gk_repository_state_enabled(session2->repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS), 0);
    assert_int_equal(gk_repository_state_enabled(session2->repository, GK_REPOSITORY_STATE_HAS_CONFLICTS), 1);
    assert_int_equal(gk_repository_state_enabled(session2->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE), 1);

    // Abort, it should clean things up
    gk_merge_abort(session2);
    assert_int_equal(gk_session_last_result_code(session2), 0);
    assert_int_equal(gk_repository_state_enabled(session2->repository, GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING), 0);
    assert_int_equal(gk_repository_state_enabled(session2->repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS), 0);
    assert_int_equal(gk_repository_state_enabled(session2->repository, GK_REPOSITORY_STATE_HAS_CONFLICTS), 0);
    assert_int_equal(gk_repository_state_enabled(session2->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE), 1);
    
    gk_session_free(session1);
    gk_session_free(session2);
}


static void test_conflicts_with_resolution_and_merge(void **state) {
    (void) state;

    // Setup repos A and B
    // A has committed and pushed various changes
    // B has committed and fetched, now has conflicts
    gk_session *session1 = NULL;
    gk_session *session2 = NULL;
    gk_test_env_conflicting_repos_a_and_b_with_extended_conflicts(&session1, &session2, state);
    
    // Attempt a merge in repo B
    gk_test_reset_state_change_record();
    gk_merge_into_head(session2);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_HAS_CONFLICTS), 1);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    // We should now have 6 conflicts of various types
    gk_merge_conflict_summary *summary = &session2->repository->conflict_summary;
    assert_int_equal(summary->num_conflicts, 6);

    // Resolve all six conflicts
    gk_conflict_resolve_accept_remote_delete(session2, "file1");
    assert_int_equal(gk_session_last_result_code(session2), 0);
    
    gk_conflict_resolve_accept_local_delete(session2, "file2");
    assert_int_equal(gk_session_last_result_code(session2), 0);

    char new_buffer[6] = "hello";
    gk_conflict_resolve_from_buffer(session2, "file3", new_buffer, 6);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    gk_conflict_resolve_from_buffer(session2, "file4", new_buffer, 6);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    gk_conflict_resolve_accept_local_delete(session2, "file5");
    assert_int_equal(gk_session_last_result_code(session2), 0);
    
    gk_conflict_resolve_accept_existing(session2, "file6", GK_CONFLICT_RESOLUTION_THEIRS);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    // We should now have 0 conflicts
    gk_merge_conflicts_query(session2);
    assert_int_equal(gk_session_last_result_code(session2), 0);
    assert_int_equal(session2->repository->conflict_summary.num_conflicts, 0);
    assert_int_equal(gk_test_state_count_disabled(GK_REPOSITORY_STATE_HAS_CONFLICTS), 1);
    assert_int_equal(gk_repository_state_enabled(session2->repository, GK_REPOSITORY_STATE_HAS_CONFLICTS), 0);
    assert_int_equal(gk_repository_state_enabled(session2->repository, GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING), 1);

    // Finalize the merge    
    gk_merge_into_head_finalize(session2);
    assert_int_not_equal(gk_session_last_result_code(session2), 1);
    
    gk_session_free(session1);
    gk_session_free(session2);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_conflicts_various_types, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_conflicts_local_delete_remote_edit_file1, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_conflicts_local_edit_remote_delete_file2, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_conflicts_incompatible_twosided_edit, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_conflicts_incompatible_twosided_create, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_conflicts_partial_resolution_causes_failed_merge, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_conflicts_with_resolution_and_merge, test_staging_clean_repo_setup),
    };

    if (directory_exists("fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
