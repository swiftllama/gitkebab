
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "git2.h"
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"
#include "gk_test_env_utils.h"


static int test_staging_setup(void **state) {
    gk_init();
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
    gk_test_env_conflicting_repos_a_and_b_with_extended_conflicts(&session1, &session2, state);
    
    // Attempt a merge in repo B
    gk_session_merge_into_head(session2);
    assert_int_equal(gk_result_code(session2->last_result), 0);

    // We should now have 6 conflicts of various types
    gk_merge_conflict_summary *summary = &session2->conflict_summary;
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
    gk_session_merge_into_head(session2);
    assert_int_equal(gk_result_code(session2->last_result), 0);

    // file1 should have a local-delete-remote-edit type conflict
    gk_merge_conflict_summary *summary = &session2->conflict_summary;
    assert_int_equal(summary->num_conflicts, 6);
    assert_string_equal(summary->conflicts[0]->path, "file1");
    assert_int_equal(summary->conflicts[0]->conflict_type, GK_MERGE_CONFLICT_LOCAL_DELETE_REMOTE_EDIT);
    
    char new_file1_path[256];
    gk_session_prepend_repository_path(session2, new_file1_path, 256, "new_file1", "prepend repo path");
    assert_int_equal(gk_result_code(session2->last_result), 0);

    gk_blob_write_contents(session2, summary->conflicts[0]->theirs_oid_id, new_file1_path, "write blob");
    assert_int_equal(gk_result_code(session2->last_result), 0);

    // Verify that the file we "preserved" is the same as our edit
    assert_int_equal(diff("test-staging/simple-repo1-A/file1", "test-staging/simple-repo1-B/new_file1"), 0);
        
    gk_conflict_resolve_accept_remote_delete(session2, "file1");
    assert_int_equal(gk_result_code(session2->last_result), 0);

    gk_session_merge_conflicts_query(session2, "query merge conflicts");
    assert_int_equal(gk_result_code(session2->last_result), 0);
    
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
    gk_session_merge_into_head(session2);
    assert_int_equal(gk_result_code(session2->last_result), 0);

    // file2 should have a local-edit-remote-delete type conflict
    gk_merge_conflict_summary *summary = &session2->conflict_summary;
    assert_int_equal(summary->num_conflicts, 6);
    assert_string_equal(summary->conflicts[1]->path, "file2");
    assert_int_equal(summary->conflicts[1]->conflict_type, GK_MERGE_CONFLICT_LOCAL_EDIT_REMOTE_DELETE);
    
    char new_file1_path[256];
    gk_session_prepend_repository_path(session2, new_file1_path, 256, "new_file2", "prepend repo path");
    assert_int_equal(gk_result_code(session2->last_result), 0);

    gk_blob_write_contents(session2, summary->conflicts[0]->theirs_oid_id, new_file1_path, "write blob");
    assert_int_equal(gk_result_code(session2->last_result), 0);

    // Verify that the file we "preserved" is the same as the incoming edit
    assert_int_equal(diff("test-staging/simple-repo1-B/file2", "test-staging/simple-repo1-B/new_file2"), 0);
        
    gk_conflict_resolve_accept_local_delete(session2, "file2");
    assert_int_equal(gk_result_code(session2->last_result), 0);

    gk_session_merge_conflicts_query(session2, "query merge conflicts");
    assert_int_equal(gk_result_code(session2->last_result), 0);
    
    assert_int_equal(summary->num_conflicts, 5);
    assert_string_equal(summary->conflicts[1]->path, "file3");


    gk_session_free(session1);
    gk_session_free(session2);
}


int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_conflicts_various_types, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_conflicts_local_delete_remote_edit_file1, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_conflicts_local_edit_remote_delete_file2, test_staging_clean_repo_setup),
    };

    if (directory_exists("src/test/fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./src/test/fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
