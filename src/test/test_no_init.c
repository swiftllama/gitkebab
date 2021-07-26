
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "gitkebab.h"

static void test_no_init_clone(void **state) {
    (void) state; /* unused */

    gk_session *session = gk_session_new("./src/test/fixtures/simple-repo1.git/", "./clone-test-1", "git", NULL, NULL);

    gk_clone(session);

    assert_int_not_equal(gk_result_code(session->last_result), 0);
    assert_string_equal(gk_result_message(session->last_result), "Gitkebab not initialized");

    gk_session_free(session);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_no_init_clone),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
