
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

static void test_merge_no_changes(void **state) {
    (void) state; /* unused */
    
    // Clone repo 
    gk_session *session1 = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1");
    assert_non_null(session1);

    gk_object_id original_head = {0};
    gk_session_resolve_reference(session1, "HEAD", &original_head);
    
    // Fetch 
    gk_session_fetch(session1, &g_empty_credential, "origin");
    assert_int_equal(gk_result_code(session1->last_result), 0);

    gk_object_id fetched_commit = {0};
    gk_session_resolve_reference(session1, "refs/remotes/origin/master", &fetched_commit);

    gk_object_id new_head_before_merge = {0};
    gk_session_resolve_reference(session1, "HEAD", &new_head_before_merge);

    // Merge
    gk_session_merge_into_head(session1, "refs/remotes/origin/master");
    assert_int_equal(gk_result_code(session1->last_result), 0);

    gk_object_id new_head_after_merge = {0};
    gk_session_resolve_reference(session1, "HEAD", &new_head_after_merge);
    
    // Compare
    assert_string_equal(original_head.id, fetched_commit.id);
    assert_string_equal(original_head.id, new_head_before_merge.id);
    assert_string_equal(original_head.id, new_head_after_merge.id);

    gk_session_free(session1);
}

static void test_merge_one_commit(void **state) {
    (void) state; /* unused */
    
    // Clone repo to two different locations
    gk_session *session1 = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1-A");
    assert_non_null(session1);

    gk_object_id repo_A_original_head = {0};
    gk_session_resolve_reference(session1, "HEAD", &repo_A_original_head);
    
    gk_session *session2 = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1-B");
    assert_non_null(session2);

    gk_object_id repo_B_original_head = {0};
    gk_session_resolve_reference(session2, "HEAD", &repo_B_original_head);
    
    // Modify repo-A, commit and push
    copy_file("src/test/fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1-A/file1");
    gk_session_index_add_path(session1, "file1");
    assert_int_equal(gk_result_code(session1->last_result), 0);
    gk_object_id repo_A_new_commit = {0};
    gk_session_commit(session1, "HEAD", "change file1", &repo_A_new_commit);
    assert_int_equal(gk_result_code(session1->last_result), 0);
    gk_session_push(session1, &g_empty_credential, "origin");
    assert_int_equal(gk_result_code(session1->last_result), 0);

    // Fetch repo-B
    gk_session_fetch(session2, &g_empty_credential, "origin");
    assert_int_equal(gk_result_code(session2->last_result), 0);
    assert_int_equal(session2->state.has_changes_to_merge, 1);
            
    gk_object_id repo_B_fetched_commit = {0};
    gk_session_resolve_reference(session2, "refs/remotes/origin/master", &repo_B_fetched_commit);
        
    // Merge in repo-B
    gk_session_merge_into_head(session2, "refs/remotes/origin/master");
    assert_int_equal(gk_result_code(session2->last_result), 0);
    assert_int_equal(session2->state.has_changes_to_merge, 0);

    gk_object_id repo_B_new_head = {0};
    gk_session_resolve_reference(session2, "HEAD", &repo_B_new_head);
    
    // Compare
    log_error(COMP_MERGE, "repo_A_original_head: %s", repo_A_original_head.id);
    log_error(COMP_MERGE, "repo_B_original_head: %s", repo_B_original_head.id);
    log_error(COMP_MERGE, "repo_A_new commit: %s", repo_A_new_commit.id);
    log_error(COMP_MERGE, "repo_B_fetched_commit: %s", repo_B_fetched_commit.id);
    log_error(COMP_MERGE, "repo_B_new_head: %s", repo_B_new_head.id);
    assert_string_equal(repo_A_original_head.id, repo_B_original_head.id);
    assert_string_not_equal(repo_A_original_head.id, repo_A_new_commit.id);
    assert_string_equal(repo_B_fetched_commit.id, repo_A_new_commit.id);
    assert_string_equal(repo_B_new_head.id, repo_A_new_commit.id);

    gk_session_free(session1);
    gk_session_free(session2);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_merge_no_changes, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_merge_one_commit, test_staging_clean_repo_setup),
    };

    if (directory_exists("src/test/fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./src/test/fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
