
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"
#include "git2.h"

#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"
#include "gk_test_env_utils.h"

const char *error_listing = "status";

static int test_staging_setup(void **state) {
    gk_init(NULL, LOG_DEBUG);
    return gk_test_environment_setup(state);

    prepare_error_listing(error_listing);
}

static int test_staging_teardown(void **state) {
    return gk_test_environment_teardown(state);
}

static int test_staging_clean_repo_setup(void **state) {
    (void) state;
    gk_test_copy_simplerepo1_from_simplerepo1_dot_gitbak();
    return 0;
}


static void test_status_without_open_repo(void **state) {
    (void) state;
    
    gk_session *session = gk_session_new("", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "./test-staging/simple-repo1", "git", NULL, NULL);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    gk_status_summary_query(session);
    assert_int_equal(gk_session_last_result_code(session), GK_ERR_REPOSITORY_NOT_INITIALIZED);
    append_to_error_listing(error_listing, "Query status without an open repo", gk_result_code_as_string(gk_session_last_result_code(session)), gk_session_last_result_message(session));

    assert_non_null(strstr(gk_session_last_result_message(session), "repository not initialized"));
    
    gk_session_free(session);
    }

static void test_status_no_changes(void **state) {
    (void) state;
    
    gk_session *session = gk_test_session_from_local_path("./test-staging/simple-repo1");
    assert_non_null(session);

    gk_status_summary_query(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    assert_int_equal(session->repository->status_summary.count_new, 0);
    assert_int_equal(session->repository->status_summary.count_modified, 0);
    assert_int_equal(session->repository->status_summary.count_deleted, 0);
    assert_int_equal(session->repository->status_summary.count_renamed, 0);
    assert_int_equal(session->repository->status_summary.count_typechange, 0);
    assert_int_equal(session->repository->status_summary.count_conflicted, 0);
    
    assert_int_equal(gk_status_summary_entrycount(session), 0);

    assert_int_equal(gk_repository_state_disabled(session->repository, GK_REPOSITORY_STATE_HAS_CONFLICTS), 1);
    assert_int_equal(gk_repository_state_disabled(session->repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS), 1);
                                
    gk_status_summary_close(session);
    gk_session_free(session);
    }

static void test_status_new_file_modified_file_deleted_file(void **state) {
    (void) state;
    
    copy_file("./test-staging/simple-repo1/file1", "./test-staging/simple-repo1/new-file");
    copy_file("./test-staging/simple-repo1/file1", "./test-staging/simple-repo1/ignored-file1");
    copy_file("./fixtures/simple-repo1-modifications/file1-modified", "./test-staging/simple-repo1/file1");
    rm_file("./test-staging/simple-repo1/file2");
    
    gk_session *session = gk_test_session_from_local_path("./test-staging/simple-repo1");
    assert_non_null(session);

    gk_status_summary_query(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    assert_int_equal(session->repository->status_summary.count_new, 1);
    assert_int_equal(session->repository->status_summary.count_modified, 1);
    assert_int_equal(session->repository->status_summary.count_deleted, 1);
    assert_int_equal(session->repository->status_summary.count_renamed, 0);
    assert_int_equal(session->repository->status_summary.count_typechange, 0);
    assert_int_equal(session->repository->status_summary.count_conflicted, 0);
    
    assert_int_equal(gk_status_summary_entrycount(session), 3);

    assert_string_equal(gk_status_summary_path_at(session, 0), "file1");
    assert_string_equal(gk_status_summary_path_at(session, 1), "file2");
    assert_string_equal(gk_status_summary_path_at(session, 2), "new-file");

    assert_int_equal(gk_status_summary_status_at(session, 0), GIT_STATUS_WT_MODIFIED);
    assert_int_equal(gk_status_summary_status_at(session, 1), GIT_STATUS_WT_DELETED);
    assert_int_equal(gk_status_summary_status_at(session, 2), GIT_STATUS_WT_NEW);

    gk_session_free(session);
}

static void test_status_renamed_file(void **state) {
    (void) state;
    
    copy_file("./test-staging/simple-repo1/file1", "./test-staging/simple-repo1/new-file");
    rm_file("./test-staging/simple-repo1/file1");
    
    gk_session *session = gk_test_session_from_local_path("./test-staging/simple-repo1");
    assert_non_null(session);

    gk_status_summary_query(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    assert_int_equal(session->repository->status_summary.count_new, 0);
    assert_int_equal(session->repository->status_summary.count_modified, 0);
    assert_int_equal(session->repository->status_summary.count_deleted, 0);
    assert_int_equal(session->repository->status_summary.count_renamed, 1);
    assert_int_equal(session->repository->status_summary.count_typechange, 0);
    assert_int_equal(session->repository->status_summary.count_conflicted, 0);
    
    assert_int_equal(gk_status_summary_entrycount(session), 1);

    assert_string_equal(gk_status_summary_path_at(session, 0), "file1");

    assert_int_equal(gk_status_summary_status_at(session, 0), GIT_STATUS_WT_RENAMED);

    gk_session_free(session);
}


int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_status_without_open_repo, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_status_no_changes, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_status_new_file_modified_file_deleted_file, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_status_renamed_file, test_staging_clean_repo_setup),
    };

    if (directory_exists("fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
