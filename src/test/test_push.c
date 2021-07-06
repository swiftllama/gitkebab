
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "git2.h"
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"

gk_session_credential_t g_empty_credential;
void session_progress(gk_session_progress_t *progress) { }


static int test_staging_setup(void **state) {
    gk_init();
        
    if (directory_exists("test-staging") == 0) {
        rm_rf("test-staging");
    }
    create_directory("test-staging");
    copy_directory("./src/test/fixtures/simple-repo1.git", "test-staging/simple-repo1.git");

    gk_session_credential_username_password_init(&g_empty_credential, "", "");
    
    return 0;
}

static int test_staging_teardown(void **state) {
    (void) state; /* unused */
    gk_session_credential_free_members(&g_empty_credential);
    
    return 0;
}

static int test_staging_clean_repo_setup(void **state) {
    (void) state;
    if (directory_exists("test-staging/simple-repo1") == 0) {
        rm_rf("test-staging/simple-repo1");
    }

    return 0;
}



static void test_push_no_changes(void **state) {
    (void) state; /* unused */
        
    gk_repository_t repo;
    gk_repository_init(&repo, "./test-staging/simple-repo1.git", "./test-staging/simple-repo1", "git");
    gk_session_t session;

    gk_session_init(&session, &repo, &session_progress);
    assert_int_equal(gk_result_code(session.last_result), 0);

    gk_session_clone(&session, &g_empty_credential);

    assert_int_equal(gk_result_code(session.last_result), 0);
    assert_int_equal(session.state.local_checkout_exists, 1);
}



int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_push_no_changes, test_staging_clean_repo_setup),
    };

    if (directory_exists("src/test/fixtures") != 0) {
        log_error(COMP_TEST, "Cannot run tests: could not find test fixtures at relative path ./src/test/fixtures");
        log_error(COMP_TEST, "Tests must be run from the project root folder");
        return 1;
    }
    
    return cmocka_run_group_tests(tests, test_staging_setup, test_staging_teardown);
}
