
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"
#include "gk_test_env_utils.h"
const char *error_listing = "clone";

static int test_staging_setup(void **state) {
    (void) state;
    
    gk_init(NULL, LOG_DEBUG);
        
    if (directory_exists("test-staging") == 0) {
        rm_rf("test-staging");
    }
        
    create_directory("test-staging");
    create_directory("test-staging/empty-dir1");
    create_directory("test-staging/nonempty-dir1");
    create_directory("test-staging/nonempty-dir1/child");

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
    if (directory_exists("test-staging/clone-test-1") == 0) {
        rm_rf("test-staging/clone-test-1");
    }
    return 0;
}

static void test_clone_simple(void **state) {
    (void) state; /* unused */
    
    gk_session *session = gk_session_new("./fixtures/simple-repo1.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "./test-staging/clone-test-1", "git", &gk_test_state_change_callback, NULL);
    gk_session_initialize(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_INITIALIZED), 1);
    
    gk_clone(session);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_CLONE_IN_PROGRESS), 1);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS), 1);
    assert_int_equal(gk_test_state_count_disabled(GK_REPOSITORY_STATE_CLONE_IN_PROGRESS), 1);
    
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS), 1);
        
    assert_int_equal(file_exists("test-staging/clone-test-1/file1"), 0);
    assert_int_equal(file_exists("test-staging/clone-test-1/file2"), 0);
    assert_int_equal(directory_exists("test-staging/clone-test-1/folder1"), 0);
    assert_int_equal(file_exists("test-staging/clone-test-1/folder1/file3"), 0);

    gk_session_free(session);
}

static void test_clone_bad_source_path(void **state) {
    (void) state; // unused
    
    gk_session *session = gk_session_new("test-staging/tmp/non-existent-path/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "./test-staging/clone-test-2", "git", NULL, NULL);
    gk_session_initialize(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    gk_clone(session);
    // Bad path gets detected before clone actually starts, state shouldn't change at all
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS), 0);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_CLONE_IN_PROGRESS), 0);
    assert_int_equal(gk_test_state_count_disabled(GK_REPOSITORY_STATE_CLONE_IN_PROGRESS), 0);
    
    assert_int_equal(gk_session_last_result_code(session), GK_ERR_CLONE_INEXISTENT_SOURCE_PATH);
    append_to_error_listing(error_listing, "Clone from source path that does not exist", gk_result_code_as_string(gk_session_last_result_code(session)), gk_session_last_result_message(session));

    gk_session_free(session);
}
static void test_clone_null_dest_path(void **state) {
    (void) state; // unused
    
    gk_session *session = gk_session_new("fixtures/simple-repo1.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, NULL, "git", NULL, NULL);
    gk_session_initialize(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    
    gk_clone(session);

    assert_int_equal(gk_session_last_result_code(session), GK_ERR_CLONE_INVALID_DESTINATION_PATH);
    append_to_error_listing(error_listing, "Clone with NULL destination path", gk_result_code_as_string(gk_session_last_result_code(session)), gk_session_last_result_message(session));

    gk_session_free(session);
}

static void test_initialize_dest_path_empty_existing_regular_dir(void **state) {
    (void) state; // unused
    
    gk_session *session = gk_session_new("fixtures/simple-repo1.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "test-staging/empty-dir1", "git", NULL, NULL);
    gk_session_initialize(session);
    assert_int_equal(gk_session_last_result_code(session), GK_ERR_REPOSITORY_LOCAL_PATH_CONFLICT);
    append_to_error_listing(error_listing, "Initialize with existing empty destination path", gk_result_code_as_string(gk_session_last_result_code(session)), gk_session_last_result_message(session));

    gk_session_free(session);
}

static void test_initialize_dest_path_nonempty_existing_regular_dir(void **state) {
    (void) state; // unused

    gk_session *session = gk_session_new("fixtures/simple-repo1.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "test-staging/nonempty-dir1", "git", NULL, NULL);
    gk_session_initialize(session);
    assert_int_equal(gk_session_last_result_code(session), GK_ERR_REPOSITORY_LOCAL_PATH_CONFLICT);
    append_to_error_listing(error_listing, "Initialize with existing non-empty destination path", gk_result_code_as_string(gk_session_last_result_code(session)), gk_session_last_result_message(session));
    
    gk_session_free(session);
}

static void test_clone_delete_clone(void **state) {
    (void) state; /* unused */
    
    gk_session *session = gk_session_new("./fixtures/simple-repo1.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "./test-staging/clone-test-1", "git", &gk_test_state_change_callback, NULL);
    gk_session_initialize(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_INITIALIZED), 1);
    assert_int_equal(gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS), 0);
    
    gk_clone(session);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_CLONE_IN_PROGRESS), 1);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS), 1);
    assert_int_equal(gk_test_state_count_disabled(GK_REPOSITORY_STATE_CLONE_IN_PROGRESS), 1);
    
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS), 1);

#if defined(_WIN32) || defined(__WIN32__)
    // NOTE: on windows can't delete a repository until it is closed/freed, this
    //       changes the nature of the test but that seems to be best
    gk_session_free(session);
#endif
    
    rm_rf("./test-staging/clone-test-1");

#if defined(_WIN32) || defined(__WIN32__)
    session = gk_session_new("./fixtures/simple-repo1.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "./test-staging/clone-test-1", "git", &gk_test_state_change_callback, NULL);
    gk_session_initialize(session);
#endif

    gk_status_summary_query(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(gk_test_state_count_disabled(GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS), 1);
    assert_int_equal(gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS), 0);

    
    gk_clone(session);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_CLONE_IN_PROGRESS), 2);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS), 2);
    assert_int_equal(gk_test_state_count_disabled(GK_REPOSITORY_STATE_CLONE_IN_PROGRESS), 2);
    
    gk_session_free(session);
}

static void test_clone_empty_repo(void **state) {
    (void) state; /* unused */

    gk_test_delete_empty_repository_dot_git();
    gk_test_copy_empty_repository_from_empty_repository_dot_gitbak();
    
    gk_session *session = gk_session_new("./test-staging/empty-repository.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "./test-staging/empty-repo-test-1", "git", &gk_test_state_change_callback, NULL);
    gk_session_initialize(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_INITIALIZED), 1);
    
    gk_clone(session);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_CLONE_IN_PROGRESS), 1);
    assert_int_equal(gk_test_state_count_enabled(GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS), 1);
    assert_int_equal(gk_test_state_count_disabled(GK_REPOSITORY_STATE_CLONE_IN_PROGRESS), 1);
    
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS), 1);

    gk_session_free(session);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_clone_simple, test_setup),
        cmocka_unit_test_setup(test_clone_bad_source_path, test_setup),
        cmocka_unit_test_setup(test_clone_null_dest_path, test_setup),
        cmocka_unit_test_setup(test_initialize_dest_path_empty_existing_regular_dir, test_setup),
        cmocka_unit_test_setup(test_initialize_dest_path_nonempty_existing_regular_dir, test_setup),
        cmocka_unit_test_setup(test_clone_delete_clone, test_setup),
        cmocka_unit_test_setup(test_clone_empty_repo, test_setup),
    };

    if (directory_exists("fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}





