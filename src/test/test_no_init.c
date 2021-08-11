
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"
#include "gitkebab.h"

static void test_no_init_clone(void **state) {
    (void) state; /* unused */

    gk_session *session = gk_session_new("./fixtures/simple-repo1.git/", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, "./clone-test-1", "git", NULL, NULL);

    gk_clone(session);

    assert_int_not_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_non_null(strstr(gk_session_last_result_message(session), "Gitkebab not initialized"));

    gk_session_free(session);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_no_init_clone),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
