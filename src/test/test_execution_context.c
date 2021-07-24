
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "gitkebab.h"
#include "gk_execution_context.h"

static void test_execution_context_new_free(void **state) {
    (void) state; /* unused */

    gk_execution_context *context = gk_execution_context_new("context test", &COMP_TEST);
    assert_non_null(context);

    assert_int_equal(gk_execution_context_stack_size(context), 1);

    gk_execution_context_free(context);
}

static void test_execution_context_push_pop(void **state) {
    (void) state; /* unused */

    gk_execution_context *context = gk_execution_context_new("context test", &COMP_TEST);
    assert_non_null(context);

    gk_execution_context_push(context, "try a subtask", NULL);
    assert_int_equal(gk_execution_context_stack_size(context), 2);

    gk_execution_context_push(context, "try yet another subtask", NULL);
    assert_int_equal(gk_execution_context_stack_size(context), 3);

    gk_execution_context_pop(context, "try yet another subtask");
    assert_int_equal(gk_execution_context_stack_size(context), 2);

    gk_execution_context_pop(context, "try a subtask");
    assert_int_equal(gk_execution_context_stack_size(context), 1);

    gk_execution_context_free(context);
}

static void test_execution_context_childless_pop_is_noop(void **state) {
    (void) state; /* unused */

    gk_execution_context *context = gk_execution_context_new("context test", &COMP_TEST);
    assert_non_null(context);

    assert_int_equal(gk_execution_context_stack_size(context), 1);
    gk_execution_context_pop(context, "try a subtask");
    assert_int_equal(gk_execution_context_stack_size(context), 1);

    gk_execution_context_free(context);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_execution_context_new_free),
        cmocka_unit_test(test_execution_context_push_pop),
        cmocka_unit_test(test_execution_context_childless_pop_is_noop),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
