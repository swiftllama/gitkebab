
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"
#include "git2.h"
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"
#include "gk_test_env_utils.h"


static int test_staging_setup(void **state);
static int test_staging_teardown(void **state);
static int test_staging_clean_repo_setup(void **state);

static int test_staging_setup(void **state) {
    gk_init(NULL, LOG_DEBUG);
    gk_libgit2_set_log_level(LOG_DEBUG);
    
    return gk_test_environment_setup(state);
}

static int test_staging_teardown(void **state) {
    (void) state; /* unused */

    return gk_test_environment_teardown(state);
}

static int test_staging_clean_repo_setup(void **state) {
    (void) state;
    gk_test_copy_simplerepo1_from_simplerepo1_dot_gitbak();
    return 0;
}


static void test_commit_count_reflog_entries(void **state) {
    (void) state;
    
    gk_session *session = gk_test_session_from_local_path("./test-staging/simple-repo1");
    assert_non_null(session);

    size_t entrycount = gk_count_reflog_entries(session, "HEAD");
    assert_int_equal(entrycount, 1); // simple-repo1 has a single commit in its initial state

    gk_session_free(session);
}

static void test_commit_no_changes(void **state) {
    (void) state;
    
    gk_session *session = gk_test_session_from_local_path("./test-staging/simple-repo1");
    assert_non_null(session);
    
    gk_commit(session, "HEAD", NULL);
    size_t entrycount = gk_count_reflog_entries(session, "HEAD");
    assert_int_equal(entrycount, 2);  // simple-repo1 has a single commit in its initial state

    gk_session_free(session);
}

static void test_commit_new_file(void **state) {
    (void) state;
    
    copy_file("test-staging/simple-repo1/file1", "test-staging/simple-repo1/new-file1");
    
    gk_session *session = gk_test_session_from_local_path("./test-staging/simple-repo1");
    assert_non_null(session);

    gk_object_id original_head_commit = {0};
    gk_resolve_reference(session, "HEAD", &original_head_commit);
    
    gk_index_add_path(session, "new-file1");

    gk_status_summary_query(session);
    assert_int_equal(gk_session_last_result_code(session), 0);
    assert_int_equal(gk_status_summary_entrycount(session), 1);
    assert_string_equal(gk_status_summary_path_at(session, 0), "new-file1");
    assert_int_equal(gk_status_summary_status_at(session, 0), GIT_STATUS_INDEX_NEW);

    gk_object_id second_commit = {0};
    gk_commit(session, "HEAD", &second_commit);

    gk_object_id new_head_commit = {0};
    gk_resolve_reference(session, "HEAD", &new_head_commit);

    assert_string_not_equal(second_commit.id, original_head_commit.id);
    assert_string_equal(second_commit.id, new_head_commit.id);
    
    gk_status_summary_query(session);
    assert_int_equal(gk_session_last_result_code(session), 0);
    assert_int_equal(gk_status_summary_entrycount(session), 0);

    size_t entrycount = gk_count_reflog_entries(session, "HEAD");
    assert_int_equal(entrycount, 2);

    gk_session_free(session);
}

static void test_commit_new_file_and_deletion_then_modification(void **state) {
    (void) state; /* unused */

    copy_file("test-staging/simple-repo1/file1", "test-staging/simple-repo1/new-file1");
    copy_file("fixtures/simple-repo1-modifications/file1-modified", "test-staging/simple-repo1/file1");
    rm_file("test-staging/simple-repo1/file2");

    gk_session *session = gk_test_session_from_local_path("./test-staging/simple-repo1");
    assert_non_null(session);

    gk_index_add_path(session, "new-file1");
    gk_index_remove_path(session, "file2");

    gk_status_summary_query(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(gk_status_summary_entrycount(session), 3);
    assert_string_equal(gk_status_summary_path_at(session, 0), "file1");
    assert_int_equal(gk_status_summary_status_at(session, 0), GIT_STATUS_WT_MODIFIED);
    assert_string_equal(gk_status_summary_path_at(session, 1), "file2");
    assert_int_equal(gk_status_summary_status_at(session, 1), GIT_STATUS_INDEX_DELETED);
    assert_string_equal(gk_status_summary_path_at(session, 2), "new-file1");
    assert_int_equal(gk_status_summary_status_at(session, 2), GIT_STATUS_INDEX_NEW);

    gk_commit(session, "HEAD", NULL);

    size_t entrycount = gk_count_reflog_entries(session, "HEAD");
    assert_int_equal(entrycount, 2);
    
    gk_status_summary_query(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(gk_status_summary_entrycount(session), 1);
    assert_string_equal(gk_status_summary_path_at(session, 0), "file1");
    assert_int_equal(gk_status_summary_status_at(session, 0), GIT_STATUS_WT_MODIFIED);

    gk_index_add_path(session, "file1");

    gk_commit(session, "HEAD", NULL);
    
    gk_status_summary_query(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(gk_status_summary_entrycount(session), 0);

    gk_session_free(session);
}

static void test_commit_in_empty_repository(void **state) {
    (void) state;
     
    gk_test_delete_empty_repository_dot_git();
    gk_test_copy_empty_repository_from_empty_repository_dot_gitbak();

    gk_session *session = gk_session_new("./test-staging/empty-repository.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "./test-staging/empty-repo-test-1", "git", &gk_test_state_change_callback, NULL);
    gk_session_initialize(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    
    gk_clone(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS), 1);

    copy_file("fixtures/simple-repo1-modifications/file1-modified", "test-staging/empty-repo-test-1/file1");
    gk_index_add_path(session, "file1");
    gk_object_id commit = {0};
    gk_commit(session, "HEAD", &commit);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    
    gk_session_free(session);
}



int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_commit_count_reflog_entries, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_commit_no_changes, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_commit_new_file, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_commit_new_file_and_deletion_then_modification, test_staging_clean_repo_setup),
        cmocka_unit_test_setup(test_commit_in_empty_repository, test_staging_clean_repo_setup)
    };

    if (directory_exists("fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
