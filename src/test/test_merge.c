
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
    gk_test_delete_simplerepo1B_mergeconflicts();
    gk_test_copy_source_repo_simplerepo1_dot_git();
    gk_test_copy_simplerepo1_from_simplerepo1B_mergeconflicts_dot_gitbak();
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

    assert_int_equal(0, gk_session_state_enabled(session1, GK_SESSION_STATE_MERGE_IN_PROGRESS));
    assert_int_equal(0, gk_session_state_enabled(session1, GK_SESSION_STATE_HAS_CONFLICTS));
    assert_int_equal(0, gk_session_state_enabled(session1, GK_SESSION_STATE_MERGE_PENDING_ON_DISK));

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
    assert_int_equal(gk_session_state_enabled(session2, GK_SESSION_STATE_HAS_CHANGES_TO_MERGE), 1);
            
    gk_object_id repo_B_fetched_commit = {0};
    gk_session_resolve_reference(session2, "refs/remotes/origin/master", &repo_B_fetched_commit);
        
    // Merge in repo-B
    gk_session_merge_into_head(session2, "refs/remotes/origin/master");
    assert_int_equal(gk_result_code(session2->last_result), 0);
    assert_int_equal(gk_session_state_disabled(session2, GK_SESSION_STATE_HAS_CHANGES_TO_MERGE), 1);

    gk_object_id repo_B_new_head = {0};
    gk_session_resolve_reference(session2, "HEAD", &repo_B_new_head);
    
    // Compare
    assert_string_equal(repo_A_original_head.id, repo_B_original_head.id);
    assert_string_not_equal(repo_A_original_head.id, repo_A_new_commit.id);
    assert_string_equal(repo_B_fetched_commit.id, repo_A_new_commit.id);
    assert_string_equal(repo_B_new_head.id, repo_A_new_commit.id);

    assert_int_equal(0, gk_session_state_enabled(session2, GK_SESSION_STATE_MERGE_IN_PROGRESS));
    assert_int_equal(0, gk_session_state_enabled(session2, GK_SESSION_STATE_HAS_CONFLICTS));
    assert_int_equal(0, gk_session_state_enabled(session2, GK_SESSION_STATE_MERGE_PENDING_ON_DISK));
    
    gk_session_free(session1);
    gk_session_free(session2);
}

static void test_merge_divergent_commits_no_conflict(void **state) {
    (void) state;
    
    // Clone repo to two different locations
    gk_session *session1 = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1-A");
    assert_non_null(session1);

    gk_session *session2 = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1-B");
    assert_non_null(session2);

    // Modify repo-A, commit and push
    gk_object_id repo_A_original_head = {0};
    gk_session_resolve_reference(session2, "HEAD", &repo_A_original_head);
    
    copy_file("src/test/fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1-A/file1");
    gk_session_index_add_path(session1, "file1");
    assert_int_equal(gk_result_code(session1->last_result), 0);
    gk_object_id repo_A_new_commit = {0};
    gk_session_commit(session1, "HEAD", "change file1", &repo_A_new_commit);
    assert_int_equal(gk_result_code(session1->last_result), 0);
    gk_session_push(session1, &g_empty_credential, "origin");
    assert_int_equal(gk_result_code(session1->last_result), 0);

    gk_object_id repo_A_new_head = {0};
    gk_session_resolve_reference(session1, "HEAD", &repo_A_new_head);

    // Modify repo-B
    gk_object_id repo_B_original_head = {0};
    gk_session_resolve_reference(session2, "HEAD", &repo_B_original_head);
    
    copy_file("src/test/fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1-B/file2");
    gk_session_index_add_path(session2, "file2");
    assert_int_equal(gk_result_code(session2->last_result), 0);
    gk_object_id repo_B_new_commit = {0};
    gk_session_commit(session2, "HEAD", "change file2", &repo_B_new_commit);
    assert_int_equal(gk_result_code(session2->last_result), 0);

    // Repo-B fetch
    gk_session_fetch(session2, &g_empty_credential, "origin");
    assert_int_equal(gk_result_code(session2->last_result), 0);

    gk_object_id repo_B_fetched_commit = {0};
    gk_session_resolve_reference(session2, "refs/remotes/origin/master", &repo_B_fetched_commit);

    gk_object_id repo_B_head_after_fetch = {0};
    gk_session_resolve_reference(session2, "HEAD", &repo_B_head_after_fetch);

    // Merge
    gk_session_merge_into_head(session2, "refs/remotes/origin/master");
    assert_int_equal(gk_result_code(session2->last_result), 0);
    assert_int_equal(gk_session_state_disabled(session2, GK_SESSION_STATE_HAS_CHANGES_TO_MERGE), 1);

    gk_object_id repo_B_head_after_merge = {0};
    gk_session_resolve_reference(session2, "HEAD", &repo_B_head_after_merge);

    // Compare
    //log_error(COMP_MERGE, "repo_A_original_head: %s", repo_A_original_head.id);
    //log_error(COMP_MERGE, "repo_B_original_head: %s", repo_B_original_head.id);
    //log_error(COMP_MERGE, "repo_A_new commit: %s", repo_A_new_commit.id);
    //log_error(COMP_MERGE, "repo_B_new commit: %s", repo_B_new_commit.id);
    //log_error(COMP_MERGE, "repo_B_fetched_commit: %s", repo_B_fetched_commit.id);
    //log_error(COMP_MERGE, "repo_B_head_after_fetch: %s", repo_B_head_after_fetch.id);
    //log_error(COMP_MERGE, "repo_B_head_after_merge: %s", repo_B_head_after_merge.id);

    assert_string_equal(repo_A_original_head.id, repo_B_original_head.id);
    assert_string_not_equal(repo_A_original_head.id, repo_A_new_commit.id);
    assert_string_not_equal(repo_B_original_head.id, repo_B_new_commit.id);
    assert_string_equal(repo_B_fetched_commit.id, repo_A_new_commit.id);
    assert_string_not_equal(repo_B_head_after_fetch.id, repo_A_new_commit.id);
    assert_string_not_equal(repo_B_head_after_merge.id, repo_A_new_commit.id);
    assert_string_not_equal(repo_B_head_after_merge.id, repo_B_new_commit.id);

    assert_int_equal(0, gk_session_state_enabled(session2, GK_SESSION_STATE_MERGE_IN_PROGRESS));
    assert_int_equal(0, gk_session_state_enabled(session2, GK_SESSION_STATE_HAS_CONFLICTS));
    assert_int_equal(0, gk_session_state_enabled(session2, GK_SESSION_STATE_MERGE_PENDING_ON_DISK));
    
    gk_session_free(session1);
    gk_session_free(session2);
}

static void test_merge_divergent_commits_with_conflict(void **state) {
    (void) state;

    // Clone repo to two different locations
    gk_session *session1 = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1-A");
    assert_non_null(session1);

    gk_session *session2 = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1-B");
    assert_non_null(session2);

    // Modify repo-A, commit and push
    gk_object_id repo_A_original_head = {0};
    gk_session_resolve_reference(session2, "HEAD", &repo_A_original_head);
    
    copy_file("src/test/fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1-A/file1");
    gk_session_index_add_path(session1, "file1");
    assert_int_equal(gk_result_code(session1->last_result), 0);
    gk_object_id repo_A_new_commit = {0};
    gk_session_commit(session1, "HEAD", "change file1", &repo_A_new_commit);
    assert_int_equal(gk_result_code(session1->last_result), 0);
    gk_session_push(session1, &g_empty_credential, "origin");
    assert_int_equal(gk_result_code(session1->last_result), 0);

    gk_object_id repo_A_new_head = {0};
    gk_session_resolve_reference(session1, "HEAD", &repo_A_new_head);

    // Modify repo-B with conflicting change, commit and push
    gk_object_id repo_B_original_head = {0};
    gk_session_resolve_reference(session2, "HEAD", &repo_B_original_head);
    
    copy_file("src/test/fixtures/simple-repo1-modifications/file1-modified-incompatible", "test-staging/simple-repo1-B/file1");
    gk_session_index_add_path(session2, "file1");
    assert_int_equal(gk_result_code(session2->last_result), 0);
    gk_object_id repo_B_new_commit = {0};
    gk_session_commit(session2, "HEAD", "change file1 incompatbile", &repo_B_new_commit);
    assert_int_equal(gk_result_code(session2->last_result), 0);

    // Repo-B fetch
    gk_session_fetch(session2, &g_empty_credential, "origin");
    assert_int_equal(gk_result_code(session2->last_result), 0);

    gk_object_id repo_B_fetched_commit = {0};
    gk_session_resolve_reference(session2, "refs/remotes/origin/master", &repo_B_fetched_commit);

    gk_object_id repo_B_head_after_fetch = {0};
    gk_session_resolve_reference(session2, "HEAD", &repo_B_head_after_fetch);

    // Merge
    gk_session_merge_into_head(session2, "refs/remotes/origin/master");

    assert_int_equal(gk_result_code(session2->last_result), 0);
    assert_int_equal(gk_session_state_enabled(session2, GK_SESSION_STATE_HAS_CHANGES_TO_MERGE), 1);

    gk_object_id repo_B_head_after_merge = {0};
    gk_session_resolve_reference(session2, "HEAD", &repo_B_head_after_merge);

    // Compare
    //log_error(COMP_MERGE, "repo_A_original_head: %s", repo_A_original_head.id);
    //log_error(COMP_MERGE, "repo_B_original_head: %s", repo_B_original_head.id);
    //log_error(COMP_MERGE, "repo_A_new commit: %s", repo_A_new_commit.id);
    //log_error(COMP_MERGE, "repo_B_new commit: %s", repo_B_new_commit.id);
    //log_error(COMP_MERGE, "repo_B_fetched_commit: %s", repo_B_fetched_commit.id);
    //log_error(COMP_MERGE, "repo_B_head_after_fetch: %s", repo_B_head_after_fetch.id);
    //log_error(COMP_MERGE, "repo_B_head_after_merge: %s", repo_B_head_after_merge.id);
    //
    assert_string_equal(repo_A_original_head.id, repo_B_original_head.id);
    assert_string_not_equal(repo_A_original_head.id, repo_A_new_commit.id);
    assert_string_not_equal(repo_B_original_head.id, repo_B_new_commit.id);
    assert_string_equal(repo_B_fetched_commit.id, repo_A_new_commit.id);
    assert_string_not_equal(repo_B_head_after_fetch.id, repo_A_new_commit.id);
    assert_string_not_equal(repo_B_head_after_merge.id, repo_A_new_commit.id);
    assert_string_equal(repo_B_head_after_merge.id, repo_B_new_commit.id); // no commit created because of conflicts

    assert_int_equal(0, gk_session_state_enabled(session2, GK_SESSION_STATE_MERGE_IN_PROGRESS));
    assert_int_equal(1, gk_session_state_enabled(session2, GK_SESSION_STATE_HAS_CONFLICTS));
    assert_int_equal(0, gk_session_state_enabled(session2, GK_SESSION_STATE_MERGE_PENDING_ON_DISK));
    
    gk_session_free(session1);
    gk_session_free(session2);
}


static void test_merge_open_existing_merge_conflicted_repo(void **state) {
    (void) state;

    // Open pre-merge conflicted repo
    gk_session *session2 = gk_test_session_from_local_path("test-staging/simple-repo1-B_merge-conflicts");

    // Compare
    assert_int_equal(0, gk_session_state_enabled(session2, GK_SESSION_STATE_MERGE_IN_PROGRESS));
    assert_int_equal(1, gk_session_state_enabled(session2, GK_SESSION_STATE_HAS_CONFLICTS));
    assert_int_equal(1, gk_session_state_enabled(session2, GK_SESSION_STATE_MERGE_PENDING_ON_DISK));

    // TODO: test resolving on-disk merge
    
    gk_session_free(session2);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_merge_no_changes, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_merge_one_commit, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_merge_divergent_commits_no_conflict, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_merge_divergent_commits_with_conflict, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_merge_open_existing_merge_conflicted_repo, test_staging_clean_repo_setup),
    };

    if (directory_exists("src/test/fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./src/test/fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
