
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "git2.h"
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"
#include "gk_test_env_utils.h"


static int test_staging_setup(void **state) {
    gk_init();
    //gk_libgit2_set_log_level(LOG_DEBUG);
    return gk_test_environment_setup(state);
}

static int test_staging_teardown(void **state) {
    return gk_test_environment_teardown(state);
}

static int test_staging_clean_repo_setup(void **state) {
    (void) state;
    gk_test_delete_simplerepo1();
    gk_test_delete_simplerepo1A();
    gk_test_delete_simplerepo1B();
    gk_test_delete_simplerepo1_dot_git();
    gk_test_copy_source_repo_simplerepo1_dot_git();
    return 0;
}

static void test_merge_no_changes(void **state) {
    (void) state; /* unused */
    
    // Clone repo 
    gk_session *session1 = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1");
    assert_non_null(session1);

    gk_object_id repo_first_commit = {0};
    gk_session_resolve_reference(session1, "HEAD", &repo_first_commit);
    
    // Fetch 
    gk_session_fetch(session1, &g_empty_credential, "origin");
    assert_int_equal(gk_result_code(session1->last_result), 0);

    gk_object_id fetched_commit = {0};
    gk_session_resolve_reference(session1, "refs/remotes/origin/master", &fetched_commit);

    gk_object_id repo_new_commit = {0};
    gk_session_resolve_reference(session1, "HEAD", &repo_new_commit);

    // Compare
    assert_string_equal(fetched_commit.id, repo_first_commit.id);
    assert_string_equal(fetched_commit.id, repo_new_commit.id);
    assert_int_equal(session1->state.has_changes_to_merge, 0);

    gk_session_free(session1);    
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_merge_no_changes, test_staging_clean_repo_setup),
    };

    if (directory_exists("src/test/fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./src/test/fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
