
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
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


static void test_status_no_changes(void **state) {
    (void) state; /* unused */

    gk_repository_t repo;
    gk_repository_init(&repo, "", "./test-staging/simple-repo1", "git");
    gk_session_t session;

    gk_session_init(&session, &repo, &session_progress);
    assert_int_equal(gk_result_code(session.last_result), 0);

    gk_session_open_local_repository(&session);
    assert_int_equal(gk_result_code(session.last_result), 0);

    gk_session_query_status_summary(&session);
    assert_int_equal(gk_result_code(session.last_result), 0);

    assert_int_equal(session.status_summary.count_new, 0);
    assert_int_equal(session.status_summary.count_modified, 0);
    assert_int_equal(session.status_summary.count_deleted, 0);
    assert_int_equal(session.status_summary.count_renamed, 0);
    assert_int_equal(session.status_summary.count_typechange, 0);
    assert_int_equal(session.status_summary.count_conflicted, 0);
    
    assert_int_equal(gk_session_status_summary_entrycount(&session), 0);
}


int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_status_no_changes, test_staging_clean_repo_setup),
    };

    if (directory_exists("src/test/fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./src/test/fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
