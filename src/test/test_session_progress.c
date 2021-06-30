
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "gitkebab.h"

static void test_checkout_init_blank(void **state) {
    (void) state; /* unused */

    gk_session_progress_t progress;
    gk_session_progress_init_checkout(&progress, NULL, 0, 0);
    assert_int_equal(progress.progress_event_type, GK_SESSION_PROGRESS_CHECKOUT);
    assert_int_equal(progress.percent, 0);
    assert_string_equal(progress.description, "");

    assert_int_equal(progress.fetch.network_percent, 0);
    assert_int_equal(progress.fetch.index_percent, 0);
    assert_int_equal(progress.fetch.received_bytes, 0);
    assert_int_equal(progress.fetch.deltas_resolved_percent, 0);

    assert_int_equal(progress.checkout.completed_steps, 0);
    assert_int_equal(progress.checkout.total_steps, 0);
    assert_int_equal(progress.checkout.checkout_percent, 0);
    assert_null(progress.checkout.current_path);
}

static void test_checkout_init_zero_percent(void **state) {
    (void) state; /* unused */

    gk_session_progress_t progress;
    gk_session_progress_init_checkout(&progress, "some-file", 0, 100);
    assert_int_equal(progress.progress_event_type, GK_SESSION_PROGRESS_CHECKOUT);
    assert_int_equal(progress.percent, 0);
    assert_string_equal(progress.description, "some-file");

    assert_int_equal(progress.checkout.completed_steps, 0);
    assert_int_equal(progress.checkout.total_steps, 100);
    assert_int_equal(progress.checkout.checkout_percent, 0);
    assert_string_equal(progress.checkout.current_path, "some-file");
}

static void test_checkout_init_ten_percent(void **state) {
    (void) state; /* unused */

    gk_session_progress_t progress;
    gk_session_progress_init_checkout(&progress, "some-file2", 10, 100);
    assert_int_equal(progress.progress_event_type, GK_SESSION_PROGRESS_CHECKOUT);
    assert_int_equal(progress.percent, 10);
    assert_string_equal(progress.description, "some-file2");
}

static void test_checkout_init_one_hundred_percent(void **state) {
    (void) state; /* unused */

    gk_session_progress_t progress;
    gk_session_progress_init_checkout(&progress, "some-file3", 100, 100);
    assert_int_equal(progress.progress_event_type, GK_SESSION_PROGRESS_CHECKOUT);
    assert_int_equal(progress.percent, 100);
    assert_string_equal(progress.description, "some-file3");
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_checkout_init_blank),
        cmocka_unit_test(test_checkout_init_zero_percent),
        cmocka_unit_test(test_checkout_init_ten_percent),
        cmocka_unit_test(test_checkout_init_one_hundred_percent),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
