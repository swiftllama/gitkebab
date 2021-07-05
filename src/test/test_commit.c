
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "git2.h"
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"

void session_progress(gk_session_progress_t *progress) { }

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


static void test_commit_count_reflog_entries(void **state) {
    (void) state; /* unused */
    
    gk_repository_t repo;
    gk_repository_init(&repo, "", "./test-staging/simple-repo1", "git");
    gk_session_t session;

    gk_session_init(&session, &repo, &session_progress);
    assert_int_equal(gk_result_code(session.last_result), 0);

    gk_session_open_local_repository(&session);
    assert_int_equal(gk_result_code(session.last_result), 0);

    size_t entrycount = gk_session_count_reflog_entries(&session, "HEAD");
    
    assert_int_equal(entrycount, 1); // simple-repo1 has a single commit in its initial state
}

static void test_commit_no_changes(void **state) {
    (void) state; /* unused */
        
    gk_repository_t repo;
    gk_repository_init(&repo, "", "./test-staging/simple-repo1", "git");
    gk_session_t session;

    gk_session_init(&session, &repo, &session_progress);
    assert_int_equal(gk_result_code(session.last_result), 0);

    gk_session_open_local_repository(&session);
    assert_int_equal(gk_result_code(session.last_result), 0);

    gk_session_commit(&session, "HEAD", "commit with no changes");

    size_t entrycount = gk_session_count_reflog_entries(&session, "HEAD");
    assert_int_equal(entrycount, 2);  // simple-repo1 has a single commit in its initial state
}

static void test_commit_new_file(void **state) {
    (void) state; /* unused */

    copy_file("test-staging/simple-repo1/file1", "test-staging/simple-repo1/new-file1");
        
    gk_repository_t repo;
    gk_repository_init(&repo, "", "./test-staging/simple-repo1", "git");
    gk_session_t session;

    gk_session_init(&session, &repo, &session_progress);
    assert_int_equal(gk_result_code(session.last_result), 0);

    gk_session_open_local_repository(&session);
    assert_int_equal(gk_result_code(session.last_result), 0);

    gk_session_index_add_path(&session, "new-file1");

    gk_session_query_status_summary(&session);
    assert_int_equal(gk_result_code(session.last_result), 0);
    assert_int_equal(gk_session_status_summary_entrycount(&session), 1);
    assert_string_equal(gk_session_status_summary_path_at(&session, 0), "new-file1");
    assert_int_equal(gk_session_status_summary_status_at(&session, 0), GIT_STATUS_INDEX_NEW);

    gk_session_commit(&session, "HEAD", "commit new file");

    gk_session_query_status_summary(&session);
    assert_int_equal(gk_result_code(session.last_result), 0);
    assert_int_equal(gk_session_status_summary_entrycount(&session), 0);

    size_t entrycount = gk_session_count_reflog_entries(&session, "HEAD");
    assert_int_equal(entrycount, 2);
}

static void test_commit_new_file_and_deletion_then_modification(void **state) {
    (void) state; /* unused */

    copy_file("test-staging/simple-repo1/file1", "test-staging/simple-repo1/new-file1");
    copy_file("src/test/fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1/file1");
    rm_rf("test-staging/simple-repo1/file2");
        
    gk_repository_t repo;
    gk_repository_init(&repo, "", "./test-staging/simple-repo1", "git");
    gk_session_t session;

    gk_session_init(&session, &repo, &session_progress);
    assert_int_equal(gk_result_code(session.last_result), 0);

    gk_session_open_local_repository(&session);
    assert_int_equal(gk_result_code(session.last_result), 0);

    gk_session_index_add_path(&session, "new-file1");
    gk_session_index_remove_path(&session, "file2");

    gk_session_query_status_summary(&session);
    assert_int_equal(gk_result_code(session.last_result), 0);
    assert_int_equal(gk_session_status_summary_entrycount(&session), 3);
    assert_string_equal(gk_session_status_summary_path_at(&session, 0), "file1");
    assert_int_equal(gk_session_status_summary_status_at(&session, 0), GIT_STATUS_WT_MODIFIED);
    assert_string_equal(gk_session_status_summary_path_at(&session, 1), "file2");
    assert_int_equal(gk_session_status_summary_status_at(&session, 1), GIT_STATUS_INDEX_DELETED);
    assert_string_equal(gk_session_status_summary_path_at(&session, 2), "new-file1");
    assert_int_equal(gk_session_status_summary_status_at(&session, 2), GIT_STATUS_INDEX_NEW);

    gk_session_commit(&session, "HEAD", "commit new file and deletion");

    size_t entrycount = gk_session_count_reflog_entries(&session, "HEAD");
    assert_int_equal(entrycount, 2);
    
    gk_session_query_status_summary(&session);
    assert_int_equal(gk_result_code(session.last_result), 0);
    assert_int_equal(gk_session_status_summary_entrycount(&session), 1);
    assert_string_equal(gk_session_status_summary_path_at(&session, 0), "file1");
    assert_int_equal(gk_session_status_summary_status_at(&session, 0), GIT_STATUS_WT_MODIFIED);

    gk_session_index_add_path(&session, "file1");

    gk_session_commit(&session, "HEAD", "commit modification");
    
    gk_session_query_status_summary(&session);
    assert_int_equal(gk_result_code(session.last_result), 0);
    assert_int_equal(gk_session_status_summary_entrycount(&session), 0);
}




int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_commit_count_reflog_entries, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_commit_no_changes, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_commit_new_file, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_commit_new_file_and_deletion_then_modification, test_staging_clean_repo_setup)
    };

    if (directory_exists("src/test/fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./src/test/fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
