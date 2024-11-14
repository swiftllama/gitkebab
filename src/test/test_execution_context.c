
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"
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

static void test_execution_context_last_unresolved(void **state) {
    (void) state; /* unused */

    gk_execution_context *context = gk_execution_context_new("context test", &COMP_TEST);
    assert_non_null(context);

    gk_execution_context_push(context, "subtask1", NULL);
    gk_execution_context_push(context, "subtask2", NULL);
    gk_execution_context_push(context, "subtask3", NULL);
    assert_int_equal(gk_execution_context_stack_size(context), 4);

    gk_execution_context *parent = NULL;
    gk_execution_context *last_unresolved = gk_execution_context_last_unresolved(context, &parent);
    assert_non_null(parent);
    assert_non_null(last_unresolved);

    assert_string_equal(parent->purpose, "subtask2");
    assert_string_equal(last_unresolved->purpose, "subtask3");

    last_unresolved->result = gk_result_new(GK_ERR, "");
    
    parent = NULL;
    last_unresolved = gk_execution_context_last_unresolved(context, &parent);
    assert_non_null(parent);
    assert_non_null(last_unresolved);

    assert_string_equal(parent->purpose, "subtask1");
    assert_string_equal(last_unresolved->purpose, "subtask2");


    last_unresolved->result = gk_result_new(GK_ERR, "");
    parent = NULL;
    last_unresolved = gk_execution_context_last_unresolved(context, &parent);
    assert_non_null(parent);
    assert_non_null(last_unresolved);

    assert_string_equal(parent->purpose, "context test");
    assert_string_equal(last_unresolved->purpose, "subtask1");
    
    gk_execution_context_free(context);
}

static void test_execution_context_push_with_resolved(void **state) {
    (void) state; /* unused */

    gk_execution_context *context = gk_execution_context_new("context test", &COMP_TEST);
    assert_non_null(context);

    gk_execution_context_push(context, "subtask1", NULL);
    gk_execution_context_push(context, "subtask2", NULL);
    gk_execution_context_push(context, "subtask3", NULL);
    assert_int_equal(gk_execution_context_stack_size(context), 4);

    gk_execution_context *parent = NULL;
    gk_execution_context *last_unresolved = gk_execution_context_last_unresolved(context, &parent);
    assert_non_null(parent);
    assert_non_null(last_unresolved);

    parent->result = gk_result_new(GK_ERR, "");
    last_unresolved->result = gk_result_new(GK_ERR, "");

    gk_execution_context_push(context, "subtask4", NULL);

    assert_int_equal(gk_execution_context_stack_size(context), 3);
    assert_string_equal(context->purpose, "context test");
    assert_string_equal(context->child_context->purpose, "subtask1");
    assert_string_equal(context->child_context->child_context->purpose, "subtask4");
    
    gk_execution_context_free(context);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_execution_context_new_free),
        cmocka_unit_test(test_execution_context_push_pop),
        cmocka_unit_test(test_execution_context_childless_pop_is_noop),
        cmocka_unit_test(test_execution_context_last_unresolved),
        cmocka_unit_test(test_execution_context_push_with_resolved),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
