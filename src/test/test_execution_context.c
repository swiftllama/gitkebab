
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "gitkebab.h"


static void test_execution_context(void **state) {
    (void) state; /* unused */

    assert_int_equal(1, 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_execution_context),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
