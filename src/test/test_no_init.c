
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "gitkebab.h"

static void test_no_init_clone(void **state) {
    (void) state; /* unused */

    gk_session *session = gk_session_new("./src/test/fixtures/simple-repo1.git/", "./clone-test-1", "git", NULL, NULL);

    gk_clone(session);

    gk_result *result_trace = gk_session_last_result_trace(session);
    assert_int_not_equal(gk_session_last_result_code(session), 0);
    assert_non_null(strstr(gk_result_message(result_trace), "Gitkebab not initialized"));

    gk_session_free(session);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_no_init_clone),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
