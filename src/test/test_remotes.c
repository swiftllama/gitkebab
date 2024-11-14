
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"
#include "git2.h"
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"
#include "gk_test_env_utils.h"


static int test_staging_setup(void **state) {
    gk_init(NULL, LOG_DEBUG);
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

    gk_test_reset_state_change_record();
    return 0;
}

static void test_push_no_changes(void **state) {
    (void) state; /* unused */

    // Open source repo, save current commit
    gk_session *session2 = gk_test_session_from_local_path("./test-staging/simple-repo1.git");
    assert_non_null(session2);

    gk_object_id source_current_commit = {0};
    gk_resolve_reference(session2, "HEAD", &source_current_commit);
    assert_int_equal(gk_session_last_result_code(session2), 0);
    
    // Clone repo, commit and push
    gk_session *session = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1");
    assert_non_null(session);
    
    gk_push(session, "origin");
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_PUSH_IN_PROGRESS), 1);
    assert_int_equal(gk_test_state_count_disabled(GK_REPOSITORY_STATE_PUSH_IN_PROGRESS), 1);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    // Verify that current commit has not changed on source repo
    gk_object_id source_last_commit = {0};
    gk_resolve_reference(session2, "HEAD", &source_last_commit);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    assert_string_equal(source_last_commit.id, source_current_commit.id);

    gk_session_free(session);
    gk_session_free(session2);
}

static void test_push_one_commit(void **state) {
    (void) state; /* unused */

    // Open source repo, save current commit
    gk_session *session2 = gk_test_session_from_local_path("./test-staging/simple-repo1.git");
    assert_non_null(session2);

    gk_object_id source_current_commit = {0};
    gk_resolve_reference(session2, "HEAD", &source_current_commit);
    assert_int_equal(gk_session_last_result_code(session2), 0);


    // Clone repo, commit and push
    gk_session *session = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1");
    assert_non_null(session);
    
    copy_file("fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1/file1");

    gk_index_add_path(session, "file1");
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    gk_object_id new_commit = {0};
    gk_commit(session, "HEAD", &new_commit);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    gk_push(session, "origin");
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_PUSH_IN_PROGRESS), 1);
    assert_int_equal(gk_test_state_count_disabled(GK_REPOSITORY_STATE_PUSH_IN_PROGRESS), 1);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    // Check new commit in source repo
    gk_object_id source_last_commit = {0};
    gk_resolve_reference(session2, "HEAD", &source_last_commit);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    assert_string_equal(new_commit.id, source_last_commit.id);
    assert_string_not_equal(new_commit.id, source_current_commit.id);

    gk_session_free(session);
    gk_session_free(session2);
}

static void test_fetch_no_changes(void **state) {
    (void) state; /* unused */
    
    // Clone repo 
    gk_session *session1 = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1");
    assert_non_null(session1);

    gk_object_id repo_first_commit = {0};
    gk_resolve_reference(session1, "HEAD", &repo_first_commit);

    // Fetch
    gk_test_reset_state_change_record();
    gk_fetch(session1, "origin");
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE), 0);
    
    assert_int_equal(gk_session_last_result_code(session1), 0);

    gk_object_id fetched_commit = {0};
    gk_resolve_reference(session1, session1->repository->spec.remote_ref_name, &fetched_commit);

    gk_object_id repo_new_commit = {0};
    gk_resolve_reference(session1, "HEAD", &repo_new_commit);

    // Compare
    assert_string_equal(fetched_commit.id, repo_first_commit.id);
    assert_string_equal(fetched_commit.id, repo_new_commit.id);
    assert_int_equal(gk_repository_state_disabled(session1->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE), 1);

    gk_session_free(session1);    
}

static void test_fetch_one_commit_with_no_push(void **state) {
    (void) state; /* unused */
    
    // Clone repo 
    gk_session *session = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1");
    assert_non_null(session);

    // Modify repo, commit but don't push
    gk_object_id original_head = {0};
    gk_resolve_reference(session, "HEAD", &original_head);

    copy_file("fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1/file1");
    gk_index_add_path(session, "file1");
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    gk_object_id new_commit = {0};
    gk_commit(session, "HEAD", &new_commit);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);


    // Fetch repo
    gk_test_reset_state_change_record();
    gk_fetch(session, "origin");
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_FETCH_IN_PROGRESS), 1);
    assert_int_equal(gk_test_state_count_disabled(GK_REPOSITORY_STATE_FETCH_IN_PROGRESS), 1);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE), 0);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    gk_object_id fetched_commit = {0};
    gk_resolve_reference(session, "refs/remotes/origin/main", &fetched_commit);

    gk_object_id new_head = {0};
    gk_resolve_reference(session, "HEAD", &new_head);

    // Compare
    assert_string_equal(fetched_commit.id, original_head.id);
    assert_string_not_equal(original_head.id, new_head.id);
    assert_string_equal(new_head.id, new_commit.id);

    assert_int_equal(gk_repository_state_disabled(session->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE), 1);

    gk_session_free(session);
}

static void test_fetch_one_commit(void **state) {
    (void) state; /* unused */
    
    // Clone repo to two different locations
    gk_session *session1 = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1-A");
    assert_non_null(session1);

    gk_session *session2 = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1-B");
    assert_non_null(session2);

    // Modify repo-A, commit and push
    gk_object_id repo_A_original_head = {0};
    gk_resolve_reference(session2, "HEAD", &repo_A_original_head);
    
    copy_file("fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1-A/file1");
    gk_index_add_path(session1, "file1");
    assert_int_equal(gk_session_last_result_code(session1), 0);
    gk_object_id new_commit = {0};
    gk_commit(session1, "HEAD", &new_commit);
    assert_int_equal(gk_session_last_result_code(session1), 0);
    gk_push(session1, "origin");
    assert_int_equal(gk_session_last_result_code(session1), 0);

    // Fetch repo-B
    gk_fetch(session2, "origin");
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_FETCH_IN_PROGRESS), 1);
    assert_int_equal(gk_test_state_count_disabled(GK_REPOSITORY_STATE_FETCH_IN_PROGRESS), 1);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE), 1);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    gk_object_id fetched_commit = {0};
    gk_resolve_reference(session2, "refs/remotes/origin/main", &fetched_commit);

    gk_object_id repo_B_first_commit = {0};
    gk_resolve_reference(session2, "HEAD", &repo_B_first_commit);

    // Compare
    assert_string_equal(repo_A_original_head.id, repo_B_first_commit.id);
    assert_string_not_equal(repo_A_original_head.id, new_commit.id);
    assert_string_equal(fetched_commit.id, new_commit.id);
    assert_int_equal(gk_repository_state_enabled(session2->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE), 1);

    gk_session_free(session1);
    gk_session_free(session2);
}


static void test_fetch_divergent_commits_no_conflict(void **state) {
    (void) state; /* unused */
    
    // Clone repo to two different locations
    gk_session *session1 = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1-A");
    assert_non_null(session1);

    gk_session *session2 = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1-B");
    assert_non_null(session2);

    // Modify repo-A, commit and push
    gk_object_id repo_A_original_head = {0};
    gk_resolve_reference(session2, "HEAD", &repo_A_original_head);
    
    copy_file("fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1-A/file1");
    gk_index_add_path(session1, "file1");
    assert_int_equal(gk_session_last_result_code(session1), 0);
    gk_object_id repo_A_new_commit = {0};
    gk_commit(session1, "HEAD", &repo_A_new_commit);
    assert_int_equal(gk_session_last_result_code(session1), 0);
    gk_push(session1, "origin");
    assert_int_equal(gk_session_last_result_code(session1), 0);

    gk_object_id repo_A_new_head = {0};
    gk_resolve_reference(session1, "HEAD", &repo_A_new_head);

    // Modify repo-B
    gk_object_id repo_B_original_head = {0};
    gk_resolve_reference(session2, "HEAD", &repo_B_original_head);
    
    copy_file("fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1-B/file2");
    gk_index_add_path(session2, "file2");
    assert_int_equal(gk_session_last_result_code(session2), 0);
    gk_object_id repo_B_new_commit = {0};
    gk_commit(session2, "HEAD", &repo_B_new_commit);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    // Repo-B fetch
    gk_fetch(session2, "origin");
    assert_int_equal(gk_session_last_result_code(session2), 0);

    gk_object_id fetched_commit = {0};
    gk_resolve_reference(session2, "refs/remotes/origin/main", &fetched_commit);

    gk_object_id repo_B_new_head = {0};
    gk_resolve_reference(session2, "HEAD", &repo_B_new_head);

    // Compare
    assert_string_equal(repo_A_original_head.id, repo_B_original_head.id);
    assert_string_equal(repo_A_new_commit.id, fetched_commit.id);
    assert_string_equal(repo_A_new_head.id, repo_A_new_commit.id);
    assert_string_equal(repo_B_new_head.id, repo_B_new_commit.id);
    assert_string_not_equal(repo_A_new_head.id, repo_B_new_head.id);
    
    gk_session_free(session1);
    gk_session_free(session2);
}

static void test_push_one_commit_in_empty_repo(void **state) {
    (void) state; /* unused */

    gk_test_delete_empty_repository_dot_git();
    gk_test_copy_empty_repository_from_empty_repository_dot_gitbak();
    
    // Clone repo, commit and push
    gk_session *session = gk_session_new("./test-staging/empty-repository.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "./test-staging/empty-repo-test-1", "git", &gk_test_state_change_callback, NULL);
    gk_session_initialize(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    
    gk_clone(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    copy_file("fixtures/simple-repo1-modifications/file1-modified", "test-staging/empty-repo-test-1/file1");
    gk_index_add_path(session, "file1");
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    gk_object_id new_commit = {0};
    gk_commit(session, "HEAD", &new_commit);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    gk_push(session, "origin");
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    /*
    // Check new commit in source repo
    gk_object_id source_last_commit = {0};
    gk_resolve_reference(session2, "HEAD", &source_last_commit);
    assert_int_equal(gk_session_last_result_code(session2), 0);

    assert_string_equal(new_commit.id, source_last_commit.id);
    assert_string_not_equal(new_commit.id, source_current_commit.id);

    gk_session_free(session2);*/
    gk_session_free(session);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_push_no_changes, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_push_one_commit, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_fetch_one_commit, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_fetch_no_changes, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_fetch_one_commit_with_no_push, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_fetch_divergent_commits_no_conflict, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_push_one_commit_in_empty_repo, test_staging_clean_repo_setup),
    };

    if (directory_exists("fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./fixtures");
        log_error(COMP_TEST, "Tests must be run from the build test folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
