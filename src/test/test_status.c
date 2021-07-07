
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "git2.h"
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"

void session_progress(gk_session_progress *progress) { }

static int test_staging_setup(void **state) {
    gk_init();
        
    if (directory_exists("test-staging") == 0) {
        rm_rf("test-staging");
    }
    create_directory("test-staging");

    return 0;
}

static int test_staging_teardown(void **state) {
    (void) state; /* unused */

    return 0;
}

static int test_staging_clean_repo_setup(void **state) {
    (void) state;
    if (directory_exists("test-staging/simple-repo1") == 0) {
        rm_rf("test-staging/simple-repo1");
    }
    copy_directory("./src/test/fixtures/simple-repo1.gitbak", "test-staging/simple-repo1");
    mv("test-staging/simple-repo1/.gitbak", "test-staging/simple-repo1/.git");    
}

static void test_status_without_open_repo(void **state) {
    (void) state; /* unused */

    gk_session *session = gk_session_new();
    gk_session_init(session, "", "./test-staging/simple-repo1", "git", &session_progress);
    assert_int_equal(gk_result_code(session->last_result), 0);

    gk_session_query_status_summary(session);
    assert_int_not_equal(gk_result_code(session->last_result), 0);
    assert_string_equal(gk_result_message(session->last_result), "Cannot query status, local checkout does not exist");

    gk_session_free(session);
}

static void test_status_no_changes(void **state) {
    (void) state; /* unused */

    gk_session *session = gk_session_new();
    gk_session_init(session, "", "./test-staging/simple-repo1", "git", &session_progress);
    assert_int_equal(gk_result_code(session->last_result), 0);

    gk_session_open_local_repository(session);
    assert_int_equal(gk_result_code(session->last_result), 0);

    gk_session_query_status_summary(session);
    assert_int_equal(gk_result_code(session->last_result), 0);

    assert_int_equal(session->status_summary.count_new, 0);
    assert_int_equal(session->status_summary.count_modified, 0);
    assert_int_equal(session->status_summary.count_deleted, 0);
    assert_int_equal(session->status_summary.count_renamed, 0);
    assert_int_equal(session->status_summary.count_typechange, 0);
    assert_int_equal(session->status_summary.count_conflicted, 0);
    
    assert_int_equal(gk_session_status_summary_entrycount(session), 0);

    gk_session_free(session);
}

static void test_status_new_file_modified_file_deleted_file(void **state) {
    (void) state; /* unused */

    copy_file("test-staging/simple-repo1/file1", "test-staging/simple-repo1/new-file");
    copy_file("test-staging/simple-repo1/file1", "test-staging/simple-repo1/ignored-file1");
    copy_file("src/test/fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1/file1");
    rm_rf("test-staging/simple-repo1/file2");
    
    gk_session *session = gk_session_new();
    gk_session_init(session, "", "./test-staging/simple-repo1", "git", &session_progress);
    assert_int_equal(gk_result_code(session->last_result), 0);

    gk_session_open_local_repository(session);
    assert_int_equal(gk_result_code(session->last_result), 0);

    gk_session_query_status_summary(session);
    assert_int_equal(gk_result_code(session->last_result), 0);

    assert_int_equal(session->status_summary.count_new, 1);
    assert_int_equal(session->status_summary.count_modified, 1);
    assert_int_equal(session->status_summary.count_deleted, 1);
    assert_int_equal(session->status_summary.count_renamed, 0);
    assert_int_equal(session->status_summary.count_typechange, 0);
    assert_int_equal(session->status_summary.count_conflicted, 0);
    
    assert_int_equal(gk_session_status_summary_entrycount(session), 3);

    assert_string_equal(gk_session_status_summary_path_at(session, 0), "file1");
    assert_string_equal(gk_session_status_summary_path_at(session, 1), "file2");
    assert_string_equal(gk_session_status_summary_path_at(session, 2), "new-file");

    assert_int_equal(gk_session_status_summary_status_at(session, 0), GIT_STATUS_WT_MODIFIED);
    assert_int_equal(gk_session_status_summary_status_at(session, 1), GIT_STATUS_WT_DELETED);
    assert_int_equal(gk_session_status_summary_status_at(session, 2), GIT_STATUS_WT_NEW);

    gk_session_free(session);
}

static void test_status_renamed_file(void **state) {
    (void) state; /* unused */

    copy_file("test-staging/simple-repo1/file1", "test-staging/simple-repo1/new-file");
    rm_rf("test-staging/simple-repo1/file1");
    
    gk_session *session = gk_session_new();
    gk_session_init(session, "", "./test-staging/simple-repo1", "git", &session_progress);
    assert_int_equal(gk_result_code(session->last_result), 0);

    gk_session_open_local_repository(session);
    assert_int_equal(gk_result_code(session->last_result), 0);

    gk_session_query_status_summary(session);
    assert_int_equal(gk_result_code(session->last_result), 0);

    assert_int_equal(session->status_summary.count_new, 0);
    assert_int_equal(session->status_summary.count_modified, 0);
    assert_int_equal(session->status_summary.count_deleted, 0);
    assert_int_equal(session->status_summary.count_renamed, 1);
    assert_int_equal(session->status_summary.count_typechange, 0);
    assert_int_equal(session->status_summary.count_conflicted, 0);
    
    assert_int_equal(gk_session_status_summary_entrycount(session), 1);

    assert_string_equal(gk_session_status_summary_path_at(session, 0), "file1");

    assert_int_equal(gk_session_status_summary_status_at(session, 0), GIT_STATUS_WT_RENAMED);

    gk_session_free(session);
}


int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_status_without_open_repo, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_status_no_changes, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_status_new_file_modified_file_deleted_file, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_status_renamed_file, test_staging_clean_repo_setup),
    };

    if (directory_exists("src/test/fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./src/test/fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
