
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "git2.h"
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"

gk_session_credential_t g_empty_credential;
void session_progress(gk_session_progress_t *progress) {
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

    // Clone repo, commit and push
    gk_repository_t repo;
    gk_repository_init(&repo, "./test-staging/simple-repo1.git", "./test-staging/simple-repo1", "git");
    gk_session_t session;

    gk_session_init(&session, &repo, &session_progress);
    assert_int_equal(gk_result_code(session.last_result), 0);
    
    gk_session_clone(&session, &g_empty_credential);
    assert_int_equal(gk_result_code(session.last_result), 0);
    assert_int_equal(session.state.local_checkout_exists, 1);
    
    gk_session_push(&session, &g_empty_credential, "origin");
    assert_int_equal(gk_result_code(session.last_result), 0);
}

static void test_push_one_commit(void **state) {
    (void) state; /* unused */

    // Clone repo, commit and push
    gk_repository_t repo;
    gk_repository_init(&repo, "./test-staging/simple-repo1.git", "./test-staging/simple-repo1", "git");
    gk_session_t session;

    gk_session_init(&session, &repo, &session_progress);
    assert_int_equal(gk_result_code(session.last_result), 0);
    
    gk_session_clone(&session, &g_empty_credential);
    assert_int_equal(gk_result_code(session.last_result), 0);
    assert_int_equal(session.state.local_checkout_exists, 1);
    
    copy_file("src/test/fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1/file1");

    gk_session_index_add_path(&session, "file1");
    assert_int_equal(gk_result_code(session.last_result), 0);
    
    gk_session_commit(&session, "HEAD", "change file1");
    assert_int_equal(gk_result_code(session.last_result), 0);

    gk_session_push(&session, &g_empty_credential, "origin");
    assert_int_equal(gk_result_code(session.last_result), 0);


    // clone repo again, verify the modified file is there
    gk_repository_t repo2;
    gk_repository_init(&repo2, "./test-staging/simple-repo1.git", "./test-staging/simple-repo2", "git");
    gk_session_t session2;

    gk_session_init(&session2, &repo2, &session_progress);
    assert_int_equal(gk_result_code(session.last_result), 0);
    
    gk_session_clone(&session2, &g_empty_credential);
    assert_int_equal(gk_result_code(session2.last_result), 0);
    assert_int_equal(session2.state.local_checkout_exists, 1);

    int diff_rc = diff("test-staging/simple-repo2/file1", "src/test/fixtures/simple-repo1-modifications/file1-modified");
    assert_int_equal(diff_rc, 0);
}



int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_push_no_changes, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_push_one_commit, test_staging_clean_repo_setup),
    };

    if (directory_exists("src/test/fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./src/test/fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
