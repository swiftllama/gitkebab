
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "gitkebab.h"

static void test_no_init_clone(void **state) {
    (void) state; /* unused */
    
    gk_repository_t repo;
    gk_repository_init(&repo, "./src/test/fixtures/simple-repo1.git/", "./clone-test-1", "git");

    gk_session_t session;
    gk_session_init(&session, &repo, NULL);

    gk_session_credential_t credential;
    gk_session_credential_username_password_init(&credential, "", "");
    gk_session_clone(&session, &credential);

    gk_session_credential_free_members(&credential);

    assert_int_not_equal(gk_result_code(session.last_result), 0);
    assert_string_equal(gk_result_message(session.last_result), "Gitkebab not initialized");
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_no_init_clone),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
