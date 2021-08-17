
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"
#include "gitkebab.h"

/**
 * Tests for the fetch/checkout progress algorithm which
 * mushes together the different internal progress values 
 * into a single percentage
 */
static void test_checkout_init_blank(void **state) {
    (void) state; /* unused */

    gk_session_progress *progress = gk_session_progress_init_checkout(NULL, 0, 0);
    assert_int_equal(progress->progress_event_type, GK_SESSION_PROGRESS_CHECKOUT);
    assert_int_equal(progress->percent, 0);
    assert_string_equal(progress->description, "");

    assert_null(progress->fetch);
    assert_null(progress->push_transfer);

    assert_int_equal(progress->checkout->completed_steps, 0);
    assert_int_equal(progress->checkout->total_steps, 0);
    assert_int_equal(progress->checkout->checkout_percent, 0);
    assert_string_equal(progress->checkout->current_path, "");
}

static void test_checkout_init_zero_percent(void **state) {
    (void) state; /* unused */

    gk_session_progress *progress = gk_session_progress_init_checkout("some-file", 0, 100);
    assert_int_equal(progress->progress_event_type, GK_SESSION_PROGRESS_CHECKOUT);
    assert_int_equal(progress->percent, 0);
    assert_string_equal(progress->description, "some-file");

    assert_int_equal(progress->checkout->completed_steps, 0);
    assert_int_equal(progress->checkout->total_steps, 100);
    assert_int_equal(progress->checkout->checkout_percent, 0);
    assert_string_equal(progress->checkout->current_path, "some-file");
}

static void test_checkout_init_ten_percent(void **state) {
    (void) state; /* unused */

    gk_session_progress *progress = gk_session_progress_init_checkout("some-file2", 10, 100);
    assert_int_equal(progress->progress_event_type, GK_SESSION_PROGRESS_CHECKOUT);
    assert_int_equal(progress->percent, 10);
    assert_string_equal(progress->description, "some-file2");
}

static void test_checkout_init_one_hundred_percent(void **state) {
    (void) state; /* unused */

    gk_session_progress *progress = gk_session_progress_init_checkout("some-file3", 100, 100);
    assert_int_equal(progress->progress_event_type, GK_SESSION_PROGRESS_CHECKOUT);
    assert_int_equal(progress->percent, 100);
    assert_string_equal(progress->description, "some-file3");
}

static void test_fetch_init_blank(void **state) {
    (void) state; /* unused */

    gk_session_progress *progress = gk_session_progress_init_fetch(0, 0, 0, 0, 0, 0);
    assert_int_equal(progress->progress_event_type, GK_SESSION_PROGRESS_FETCH);
    assert_int_equal(progress->percent, 0);
    assert_string_equal(progress->description, "Receiving objects");

    assert_int_equal(progress->fetch->network_percent, 0);
    assert_int_equal(progress->fetch->index_percent, 0);
    assert_int_equal(progress->fetch->received_bytes, 0);
    assert_int_equal(progress->fetch->deltas_resolved_percent, 0);

    assert_null(progress->checkout);
    assert_null(progress->push_transfer);
}

static void test_fetch_init_zero_percent(void **state) {
    (void) state; /* unused */

    gk_session_progress *progress = gk_session_progress_init_fetch(0, 100, 20, 0, 0, 0);
    assert_int_equal(progress->progress_event_type, GK_SESSION_PROGRESS_FETCH);
    assert_int_equal(progress->percent, 0);
    assert_string_equal(progress->description, "Receiving objects");

    assert_int_equal(progress->fetch->network_percent, 0);
    assert_int_equal(progress->fetch->index_percent, 0);
    assert_int_equal(progress->fetch->received_bytes, 0);
    assert_int_equal(progress->fetch->deltas_resolved_percent, 0);
}

static void test_fetch_init_ten_percent_receiving(void **state) {
    (void) state; /* unused */

    gk_session_progress *progress = gk_session_progress_init_fetch(123, 100, 20, 10, 0, 0);
    assert_int_equal(progress->progress_event_type, GK_SESSION_PROGRESS_FETCH);
    assert_int_equal(progress->percent, 3); // two thirds of half of ten percent
    assert_string_equal(progress->description, "Receiving objects");

    assert_int_equal(progress->fetch->network_percent, 10);
    assert_int_equal(progress->fetch->index_percent, 0);
    assert_int_equal(progress->fetch->received_bytes, 123);
    assert_int_equal(progress->fetch->deltas_resolved_percent, 0);
}

static void test_fetch_init_fifty_percent_receiving_fifty_percent_indexing(void **state) {
    (void) state; /* unused */

    gk_session_progress *progress = gk_session_progress_init_fetch(0, 100, 20, 50, 50, 0);
    assert_int_equal(progress->progress_event_type, GK_SESSION_PROGRESS_FETCH);
    assert_int_equal(progress->percent, 33); // two thirds of fifty percent
    assert_string_equal(progress->description, "Receiving objects");

    assert_int_equal(progress->fetch->network_percent, 50);
    assert_int_equal(progress->fetch->index_percent, 50);
    assert_int_equal(progress->fetch->received_bytes, 0);
    assert_int_equal(progress->fetch->deltas_resolved_percent, 0);
}

static void test_fetch_init_one_hundred_percent_receiving_fifty_percent_indexing(void **state) {
    (void) state; /* unused */

    gk_session_progress *progress = gk_session_progress_init_fetch(0, 100, 20, 100, 50, 0);
    assert_int_equal(progress->progress_event_type, GK_SESSION_PROGRESS_FETCH);
    assert_int_equal(progress->percent, 49); // one third of 100 percent from receiving + one third of 50 from indexing
    assert_string_equal(progress->description, "Indexing objects");

    assert_int_equal(progress->fetch->network_percent, 100);
    assert_int_equal(progress->fetch->index_percent, 50);
    assert_int_equal(progress->fetch->received_bytes, 0);
    assert_int_equal(progress->fetch->deltas_resolved_percent, 0);
}

static void test_fetch_init_one_hundred_percent_receiving_one_hundred_percent_indexing_zero_deltas(void **state) {
    (void) state; /* unused */

    gk_session_progress *progress = gk_session_progress_init_fetch(0, 100, 20, 100, 100, 0);
    assert_int_equal(progress->progress_event_type, GK_SESSION_PROGRESS_FETCH);
    assert_int_equal(progress->percent, 67); // algorithm fixes resolving to start at 67 percent
    assert_string_equal(progress->description, "Resolving deltas");

    assert_int_equal(progress->fetch->network_percent, 100);
    assert_int_equal(progress->fetch->index_percent, 100);
    assert_int_equal(progress->fetch->received_bytes, 0);
    assert_int_equal(progress->fetch->deltas_resolved_percent, 0);
}

static void test_fetch_init_one_hundred_percent_receiving_one_hundred_percent_indexing_half_deltas(void **state) {
    (void) state; /* unused */

    gk_session_progress *progress = gk_session_progress_init_fetch(0, 100, 20, 100, 100, 10);
    assert_int_equal(progress->progress_event_type, GK_SESSION_PROGRESS_FETCH);
    assert_int_equal(progress->percent, 67+16); // 67 from receiving+indexing objects, half of 33 from indexing deltas
    assert_string_equal(progress->description, "Resolving deltas");

    assert_int_equal(progress->fetch->network_percent, 100);
    assert_int_equal(progress->fetch->index_percent, 100);
    assert_int_equal(progress->fetch->received_bytes, 0);
    assert_int_equal(progress->fetch->deltas_resolved_percent, 50);
}

static void test_fetch_init_one_hundred_percent_all(void **state) {
    (void) state; /* unused */

    gk_session_progress *progress = gk_session_progress_init_fetch(0, 100, 20, 100, 100, 20);
    assert_int_equal(progress->progress_event_type, GK_SESSION_PROGRESS_FETCH);
    assert_int_equal(progress->percent, 100); 
    assert_string_equal(progress->description, "Resolving deltas");

    assert_int_equal(progress->fetch->network_percent, 100);
    assert_int_equal(progress->fetch->index_percent, 100);
    assert_int_equal(progress->fetch->received_bytes, 0);
    assert_int_equal(progress->fetch->deltas_resolved_percent, 100);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_checkout_init_blank),
        cmocka_unit_test(test_checkout_init_zero_percent),
        cmocka_unit_test(test_checkout_init_ten_percent),
        cmocka_unit_test(test_checkout_init_one_hundred_percent),
        cmocka_unit_test(test_fetch_init_blank),
        cmocka_unit_test(test_fetch_init_zero_percent),
        cmocka_unit_test(test_fetch_init_ten_percent_receiving),
        cmocka_unit_test(test_fetch_init_fifty_percent_receiving_fifty_percent_indexing),
        cmocka_unit_test(test_fetch_init_one_hundred_percent_receiving_fifty_percent_indexing),
        cmocka_unit_test(test_fetch_init_one_hundred_percent_receiving_one_hundred_percent_indexing_zero_deltas),
        cmocka_unit_test(test_fetch_init_one_hundred_percent_receiving_one_hundred_percent_indexing_half_deltas),
        cmocka_unit_test(test_fetch_init_one_hundred_percent_all)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
