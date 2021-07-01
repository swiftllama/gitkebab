
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"

void session_progress(gk_session_progress_t *progress) { }

gk_session_credential_t g_empty_credential;

static int test_staging_setup(void **state) {
    gk_init();
        
    if (directory_exists("test-staging") == 0) {
        log_warn(COMP_TEST, "test staging directory 'test-staging' found to exist during test setup, this could mean a previous test was aborted mid-way. Removing");
        rm_rf("test-staging");
    }
        
    create_directory("test-staging");
    create_directory("test-staging/empty-dir1");
    create_directory("test-staging/nonempty-dir1");
    create_directory("test-staging/nonempty-dir1/child");
    
    gk_session_credential_username_password_init(&g_empty_credential, "", "");

    return 0;
}

static int test_staging_teardown(void **state) {
    (void) state; /* unused */
    gk_session_credential_free_members(&g_empty_credential);

    rm_rf("test-staging");

    return 0;
}


static void test_clone_simple(void **state) {
    (void) state; /* unused */
    
    gk_repository_t repo;
    gk_repository_init(&repo, "./src/test/fixtures/simple-repo1.git/", "./test-staging/clone-test-1", "git");
    gk_session_t session;
    gk_session_init(&session, &repo, &session_progress);
    gk_session_clone(&session, &g_empty_credential);

    assert_int_equal(file_exists("test-staging/clone-test-1/file1"), 0);
    assert_int_equal(file_exists("test-staging/clone-test-1/file2"), 0);
    assert_int_equal(directory_exists("test-staging/clone-test-1/folder1"), 0);
    assert_int_equal(file_exists("test-staging/clone-test-1/folder1/file3"), 0);
}

static void test_clone_bad_source_path(void **state) {
    (void) state; /* unused */
    
    gk_repository_t repo;
    gk_repository_init(&repo, "test-staging/tmp/non-existent-path/", "./test-staging/clone-test-2", "git");
    gk_session_t session;
    gk_session_init(&session, &repo, &session_progress);
    gk_session_clone(&session, &g_empty_credential);

    assert_int_not_equal(gk_result_code(session.last_result), 0);
}

static void test_clone_null_dest_path(void **state) {
    (void) state; /* unused */
    
    gk_repository_t repo;
    gk_repository_init(&repo, "src/test/fixtures/simple-repo1.git/", NULL, "git");
    gk_session_t session;
    gk_session_init(&session, &repo, &session_progress);
    gk_session_clone(&session, &g_empty_credential);

    assert_int_not_equal(gk_result_code(session.last_result), 0);
}

static void test_clone_dest_path_empty_existing_regular_dir(void **state) {
    (void) state; /* unused */

    gk_repository_t repo;
    gk_repository_init(&repo, "src/test/fixtures/simple-repo1.git/", "test-staging/empty-dir1", "git");
    gk_session_t session;
    gk_session_init(&session, &repo, &session_progress);
    gk_session_clone(&session, &g_empty_credential);

    assert_int_equal(gk_result_code(session.last_result), 0);
}

static void test_clone_dest_path_nonempty_existing_regular_dir(void **state) {
    (void) state; /* unused */

    gk_repository_t repo;
    gk_repository_init(&repo, "src/test/fixtures/simple-repo1.git/", "test-staging/nonempty-dir1", "git");
    gk_session_t session;
    gk_session_init(&session, &repo, &session_progress);
    gk_session_clone(&session, &g_empty_credential);

    assert_int_not_equal(gk_result_code(session.last_result), 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_clone_simple),
        cmocka_unit_test(test_clone_bad_source_path),
        cmocka_unit_test(test_clone_null_dest_path),
        cmocka_unit_test(test_clone_dest_path_empty_existing_regular_dir),
        cmocka_unit_test(test_clone_dest_path_nonempty_existing_regular_dir)
    };

    if (directory_exists("src/test/fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./src/test/fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
