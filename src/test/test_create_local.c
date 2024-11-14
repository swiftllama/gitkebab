
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
    
    gk_init(NULL, LOG_DEBUG);

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

static void test_local_create_with_remotes(void **state) {
    (void) state; /* unused */

    gk_session *session = gk_session_new("", GK_REPOSITORY_SOURCE_URL_SSH, "./test-staging/create-local-2", "", NULL, NULL);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    gk_session_initialize(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    gk_create_local_repository(session, "main");
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    gk_remote_list *remotes = gk_remotes_list(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(remotes->count, 0);
    gk_remote_list_free(remotes);

    gk_remote_create(session, "origin1", "ssh://www.example.com/1");
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    remotes = gk_remotes_list(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(remotes->count, 1);
    assert_string_equal(remotes->remotes[0]->name, "origin1");
    assert_string_equal(remotes->remotes[0]->url, "ssh://www.example.com/1");    
    gk_remote_list_free(remotes);


    gk_remote_create(session, "origin2", "ssh://www.example.com/2");
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    remotes = gk_remotes_list(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(remotes->count, 2);
    assert_string_equal(remotes->remotes[0]->name, "origin1");
    assert_string_equal(remotes->remotes[0]->url, "ssh://www.example.com/1");    
    assert_string_equal(remotes->remotes[1]->name, "origin2");
    assert_string_equal(remotes->remotes[1]->url, "ssh://www.example.com/2");
    gk_remote_list_free(remotes);

    gk_remote_delete(session, "origin1");
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);

    remotes = gk_remotes_list(session);
    assert_int_equal(gk_session_last_result_code(session), GK_SUCCESS);
    assert_int_equal(remotes->count, 1);
    assert_string_equal(remotes->remotes[0]->name, "origin2");
    assert_string_equal(remotes->remotes[0]->url, "ssh://www.example.com/2");
    gk_remote_list_free(remotes);
    
    gk_session_free(session);
    
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_create_local_repository),
        cmocka_unit_test(test_local_create_with_remotes),
    };
    return cmocka_run_group_tests(tests, test_staging_setup, NULL);
}

