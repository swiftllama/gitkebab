
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"
#include "gk_test_env_utils.h"
const char *error_listing = "sync";

static int test_staging_setup(void **state) {
    (void) state;
    
    gk_init(NULL, LOG_DEBUG);
        
    if (directory_exists("test-staging") == 0) {
        rm_rf("test-staging");
    }
        
    create_directory("test-staging");

    prepare_error_listing(error_listing);
    
    return 0;
}

static int test_staging_teardown(void **state) {
    (void) state; // unused

    return 0;
}

static int test_setup(void **state) {
    (void) state;
    gk_test_reset_state_change_record();
    gk_test_delete_empty_repo_test_1();
    gk_test_delete_empty_repo_test_2();
    gk_test_delete_empty_repository_dot_git();
    gk_test_copy_empty_repository_from_empty_repository_dot_gitbak();
    
    return 0;
}

static void test_sync_empty_repository(void **state) {
    (void) state; /* unused */
    
    gk_session *session = gk_session_new("./test-staging/empty-repository.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "./test-staging/empty-repo-test-1", "git", &gk_test_state_change_callback, NULL);
    gk_session_initialize(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    
    gk_sync(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    gk_session_free(session);
}

static void test_sync_empty_repository_twice(void **state) {
    (void) state; /* unused */
    
    gk_session *session = gk_session_new("./test-staging/empty-repository.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "./test-staging/empty-repo-test-1", "git", &gk_test_state_change_callback, NULL);
    gk_session_initialize(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    
    gk_sync(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    gk_sync(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    
    gk_session_free(session);
}

static void test_sync_empty_repository_add_commit(void **state) {
    (void) state; 
    
    gk_session *session = gk_session_new("./test-staging/empty-repository.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "./test-staging/empty-repo-test-1", "git", &gk_test_state_change_callback, NULL);
    gk_session_initialize(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    
    gk_sync(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    copy_file("fixtures/simple-repo1-modifications/file1-modified", "test-staging/empty-repo-test-1/file1");

    gk_sync(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    
    gk_session_free(session);
}

static void test_sync_two_empty_repositories_exchange_commits(void **state) {
    (void) state; 

    // Sync first repository
    gk_session *session1 = gk_session_new("./test-staging/empty-repository.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "./test-staging/empty-repo-test-1", "git", &gk_test_state_change_callback, NULL);
    gk_session_initialize(session1);
    assert_int_equal(gk_session_last_result_code(session1), GK_SUCCESS);
    
    gk_sync(session1);
    assert_int_equal(gk_session_last_result_code(session1), GK_SUCCESS);

    // Sync second repository
    gk_session *session2 = gk_session_new("./test-staging/empty-repository.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "./test-staging/empty-repo-test-2", "git", &gk_test_state_change_callback, NULL);
    gk_session_initialize(session2);
    assert_int_equal(gk_session_last_result_code(session2), GK_SUCCESS);
    
    gk_sync(session2);
    assert_int_equal(gk_session_last_result_code(session2), GK_SUCCESS);
    
    copy_file("fixtures/simple-repo1-modifications/file1-modified", "test-staging/empty-repo-test-1/file1");
    gk_sync(session1);
    assert_int_equal(gk_session_last_result_code(session1), GK_SUCCESS);

    gk_sync(session2);
    assert_int_equal(gk_session_last_result_code(session2), GK_SUCCESS);

    copy_file("fixtures/simple-repo1-modifications/file1-modified", "test-staging/empty-repo-test-2/file2");

    gk_sync(session2);
    assert_int_equal(gk_session_last_result_code(session2), GK_SUCCESS);

    gk_sync(session1);
    assert_int_equal(gk_session_last_result_code(session1), GK_SUCCESS);
    
    gk_session_free(session1);
    gk_session_free(session2);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_sync_empty_repository, test_setup),
        cmocka_unit_test_setup(test_sync_empty_repository_twice, test_setup),
        cmocka_unit_test_setup(test_sync_empty_repository_add_commit, test_setup),
        cmocka_unit_test_setup(test_sync_two_empty_repositories_exchange_commits, test_setup),
    };

    if (directory_exists("fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}





