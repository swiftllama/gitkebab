
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"
#include "gitkebab.h"
#include "gk_test_filesystem_utils.h"
#include "gk_test_env_utils.h"

static int test_staging_setup(void **state) {
    (void) state;
    
    gk_init();

    if (directory_exists("test-staging") == 0) {
        rm_rf("test-staging");
    }
    create_directory("test-staging");
    
    return 0;
}


static void test_create_local_repository(void **state) {
    (void) state; /* unused */

    gk_session *session = gk_session_new("", GK_REPOSITORY_SOURCE_URL_SSH, "./test-staging/create-local-1", "", NULL, NULL);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    gk_session_initialize(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    gk_create_local_repository(session, "main");
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    gk_session_free(session);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_create_local_repository),
    };
    return cmocka_run_group_tests(tests, test_staging_setup, NULL);
}

