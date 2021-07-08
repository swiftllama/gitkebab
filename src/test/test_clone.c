
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"

gk_session_credential g_empty_credential;
void session_progress(gk_session_progress *progress) { }

static int test_staging_setup(void **state) {
    gk_init();
        
    if (directory_exists("test-staging") == 0) {
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

    return 0;
}


static void test_clone_simple(void **state) {
    (void) state; /* unused */
    
    gk_session *session = gk_session_new();
    gk_session_init(session, "./src/test/fixtures/simple-repo1.git/", "./test-staging/clone-test-1", "git", &session_progress, NULL);
    gk_session_clone(session, &g_empty_credential);

    assert_int_equal(gk_result_code(session->last_result), 0);
    assert_int_equal(session->state.local_checkout_exists, 1);
        
    assert_int_equal(file_exists("test-staging/clone-test-1/file1"), 0);
    assert_int_equal(file_exists("test-staging/clone-test-1/file2"), 0);
    assert_int_equal(directory_exists("test-staging/clone-test-1/folder1"), 0);
    assert_int_equal(file_exists("test-staging/clone-test-1/folder1/file3"), 0);

    gk_session_free(session);
}

static void test_clone_bad_source_path(void **state) {
    (void) state; /* unused */
    
    gk_session *session = gk_session_new();
    gk_session_init(session, "test-staging/tmp/non-existent-path/", "./test-staging/clone-test-2", "git", &session_progress, NULL);
    gk_session_clone(session, &g_empty_credential);

    assert_int_not_equal(gk_result_code(session->last_result), 0);

    gk_session_free(session);
}

static void test_clone_null_dest_path(void **state) {
    (void) state; /* unused */
    
    gk_session *session = gk_session_new();
    gk_session_init(session, "src/test/fixtures/simple-repo1.git/", NULL, "git", &session_progress, NULL);
    gk_session_clone(session, &g_empty_credential);

    assert_int_not_equal(gk_result_code(session->last_result), 0);

    gk_session_free(session);
}

static void test_clone_dest_path_empty_existing_regular_dir(void **state) {
    (void) state; /* unused */
    
    gk_session *session = gk_session_new();
    gk_session_init(session, "src/test/fixtures/simple-repo1.git/", "test-staging/empty-dir1", "git", &session_progress, NULL);
    gk_session_clone(session, &g_empty_credential);

    assert_int_equal(gk_result_code(session->last_result), 0);

    gk_session_free(session);
}

static void test_clone_dest_path_nonempty_existing_regular_dir(void **state) {
    (void) state; /* unused */

    gk_session *session = gk_session_new();
    gk_session_init(session, "src/test/fixtures/simple-repo1.git/", "test-staging/nonempty-dir1", "git", &session_progress, NULL);
    gk_session_clone(session, &g_empty_credential);

    assert_int_not_equal(gk_result_code(session->last_result), 0);

    gk_session_free(session);
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
