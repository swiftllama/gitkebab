
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "git2.h"
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"

gk_session_credential g_empty_credential;
void session_progress(gk_session_progress *progress) {
    if (progress != NULL) {
        log_info(COMP_TEST, "PROGRESS [%s] (%zu%%)", progress->description, progress->percent);
    }
    else {
        log_error(COMP_TEST, "Error in progress callback, callback invoked with NULL progress struct");
    }
}


static int test_staging_setup(void **state) {
    gk_init();
    gk_libgit2_set_log_level(LOG_DEBUG);
    
    if (directory_exists("test-staging") == 0) {
        rm_rf("test-staging");
    }
    create_directory("test-staging");

    gk_session_credential_username_password_init(&g_empty_credential, "", "");
    
    return 0;
}

static int test_staging_teardown(void **state) {
    (void) state; /* unused */
    gk_session_credential_free_members(&g_empty_credential);
    
    return 0;
}

static int test_staging_clean_repo_setup(void **state) {
    (void) state;
    if (directory_exists("test-staging/simple-repo1") == 0) {
        rm_rf("test-staging/simple-repo1");
    }
    if (directory_exists("test-staging/simple-repo1.git") == 0) {
        rm_rf("test-staging/simple-repo1");
    }
    copy_directory("./src/test/fixtures/simple-repo1.git", "test-staging/simple-repo1.git");

    return 0;
}

static void test_push_no_changes(void **state) {
    (void) state; /* unused */

    // Open source repo, save current commit
    gk_session *session2 = gk_session_new();
    gk_session_init(session2, "", "./test-staging/simple-repo1.git", "git", &session_progress);
    assert_int_equal(gk_result_code(session2->last_result), 0);

    gk_session_open_local_repository(session2);
    assert_int_equal(gk_result_code(session2->last_result), 0);

    gk_object_id source_current_commit = {0};
    gk_session_resolve_reference(session2, "HEAD", &source_current_commit);
    assert_int_equal(gk_result_code(session2->last_result), 0);
    
    // Clone repo, commit and push
    gk_session *session = gk_session_new();
    gk_session_init(session, "./test-staging/simple-repo1.git", "./test-staging/simple-repo1", "git", &session_progress);
    assert_int_equal(gk_result_code(session->last_result), 0);
    
    gk_session_clone(session, &g_empty_credential);
    assert_int_equal(gk_result_code(session->last_result), 0);
    assert_int_equal(session->state.local_checkout_exists, 1);
    
    gk_session_push(session, &g_empty_credential, "origin");
    assert_int_equal(gk_result_code(session->last_result), 0);

    // Verify that current commit has not changed on source repo
    gk_object_id source_last_commit = {0};
    gk_session_resolve_reference(session2, "HEAD", &source_last_commit);
    assert_int_equal(gk_result_code(session2->last_result), 0);

    assert_string_equal(source_last_commit.id, source_current_commit.id);

    gk_session_free(session);
    gk_session_free(session2);
}

static void test_push_one_commit(void **state) {
    (void) state; /* unused */

    // Open source repo, save current commit
    gk_session *session2 = gk_session_new();
    gk_session_init(session2, "", "./test-staging/simple-repo1.git", "git", &session_progress);
    assert_int_equal(gk_result_code(session2->last_result), 0);

    gk_session_open_local_repository(session2);
    assert_int_equal(gk_result_code(session2->last_result), 0);

    gk_object_id source_current_commit = {0};
    gk_session_resolve_reference(session2, "HEAD", &source_current_commit);
    assert_int_equal(gk_result_code(session2->last_result), 0);


    // Clone repo, commit and push
    gk_session *session = gk_session_new();
    gk_session_init(session, "./test-staging/simple-repo1.git", "./test-staging/simple-repo1", "git", &session_progress);
    assert_int_equal(gk_result_code(session->last_result), 0);
    
    gk_session_clone(session, &g_empty_credential);
    assert_int_equal(gk_result_code(session->last_result), 0);
    assert_int_equal(session->state.local_checkout_exists, 1);
    
    copy_file("src/test/fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1/file1");

    gk_session_index_add_path(session, "file1");
    assert_int_equal(gk_result_code(session->last_result), 0);

    gk_object_id new_commit = {0};
    gk_session_commit(session, "HEAD", "change file1", &new_commit);
    assert_int_equal(gk_result_code(session->last_result), 0);

    gk_session_push(session, &g_empty_credential, "origin");
    assert_int_equal(gk_result_code(session->last_result), 0);

    // Check new commit in source repo
    gk_object_id source_last_commit = {0};
    gk_session_resolve_reference(session2, "HEAD", &source_last_commit);
    assert_int_equal(gk_result_code(session2->last_result), 0);

    assert_string_equal(new_commit.id, source_last_commit.id);
    assert_string_not_equal(new_commit.id, source_current_commit.id);

    gk_session_free(session);
    gk_session_free(session2);
}


static void test_fetch_one_commit(void **state) {
    (void) state; /* unused */
    
    // Clone repo to two different locations
    gk_session *session1 = gk_session_new();
    gk_session_init(session1, "./test-staging/simple-repo1.git", "./test-staging/simple-repo1-A", "git", &session_progress);
    assert_int_equal(gk_result_code(session1->last_result), 0);
    gk_session_clone(session1, &g_empty_credential);
    assert_int_equal(gk_result_code(session1->last_result), 0);
    assert_int_equal(session1->state.local_checkout_exists, 1);
    

    gk_session *session2 = gk_session_new();
    gk_session_init(session2, "./test-staging/simple-repo1.git", "./test-staging/simple-repo1-B", "git", &session_progress);
    assert_int_equal(gk_result_code(session2->last_result), 0);
    gk_session_clone(session2, &g_empty_credential);
    assert_int_equal(gk_result_code(session2->last_result), 0);
    assert_int_equal(session2->state.local_checkout_exists, 1);

    // Modify repo-A, commit and push
    gk_object_id repo_A_first_commit = {0};
    gk_session_resolve_reference(session2, "HEAD", &repo_A_first_commit);
    
    copy_file("src/test/fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1-A/file1");
    gk_session_index_add_path(session1, "file1");
    assert_int_equal(gk_result_code(session1->last_result), 0);
    gk_object_id new_commit = {0};
    gk_session_commit(session1, "HEAD", "change file1", &new_commit);
    assert_int_equal(gk_result_code(session1->last_result), 0);
    gk_session_push(session1, &g_empty_credential, "origin");
    assert_int_equal(gk_result_code(session1->last_result), 0);

    // Fetch repo-B
    gk_session_fetch(session2, &g_empty_credential, "origin");
    assert_int_equal(gk_result_code(session2->last_result), 0);

    gk_object_id fetched_commit = {0};
    gk_session_resolve_reference(session2, "refs/remotes/origin/master", &fetched_commit);

    gk_object_id repo_B_first_commit = {0};
    gk_session_resolve_reference(session2, "HEAD", &repo_B_first_commit);

    assert_string_equal(repo_A_first_commit.id, repo_B_first_commit.id);
    assert_string_not_equal(repo_A_first_commit.id, new_commit.id);
    assert_string_equal(fetched_commit.id, new_commit.id);

    gk_session_free(session1);
    gk_session_free(session2);
    
}

int main(void) {
    const struct CMUnitTest tests[] = {
        //cmocka_unit_test_setup(test_push_no_changes, test_staging_clean_repo_setup),
        //cmocka_unit_test_setup(test_push_one_commit, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_fetch_one_commit, test_staging_clean_repo_setup),
    };

    if (directory_exists("src/test/fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./src/test/fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
