
#include "gk_test_env_utils.h"
#include "gk_test_filesystem_utils.h"
#include "gitkebab.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"

int g_repository_state_record_enabled[GK_REPOSITORY_STATE_MAX_EXP];
int g_repository_state_record_disabled[GK_REPOSITORY_STATE_MAX_EXP];
gk_repository_state g_old_state = 0;
gk_repository_state g_current_state = 0;

void gk_test_reset_state_change_record() {
    for (int i = 0;  i < GK_REPOSITORY_STATE_MAX_EXP; i += 1) {
        g_repository_state_record_enabled[i] = 0;
        g_repository_state_record_disabled[i] = 0;
    }
    g_old_state = GK_REPOSITORY_STATE_DEFAULT;
    g_current_state = GK_REPOSITORY_STATE_DEFAULT;
}

void gk_test_state_change_callback(const char *session_id, gk_repository *repository, gk_session_progress *progress) {
    (void)session_id;
    g_old_state = g_current_state;
    g_current_state = repository->state;
    //log_error(COMP_TEST, "[STATE] state changed, old [%d] new [%d]", g_old_state, g_current_state);
    for (int i = 0;  i < GK_REPOSITORY_STATE_MAX_EXP; i += 1) {
        int bitmask = 1 << i;
        int was_on = (bitmask & g_old_state);
        int is_on = (bitmask & g_current_state);
        //log_error(COMP_TEST, "[STATE] [%d] was on [%d] is_on [%d]", bitmask, was_on, is_on);
        if ((was_on != 0) && (is_on == 0)) {
            g_repository_state_record_disabled[i] += 1;
        }
        if ((was_on == 0) && (is_on != 0)) {
            g_repository_state_record_enabled[i] += 1;
        }
    }

    if (progress != NULL) {
        log_info(COMP_TEST, "PROGRESS session [%s] [%s] (%zu%%)", session_id, progress->description, progress->percent);
    }
    else {
        //log_error(COMP_TEST, "Error in session state (progress) callback, callback invoked with NULL progress struct");
    }
}

int gk_test_state_count_enabled(gk_repository_state state) {
    for (int i = 0; i < GK_REPOSITORY_STATE_MAX_EXP; i += 1) {
        if ((unsigned int)(1 << i) == state) {
            return g_repository_state_record_enabled[i];
        }
    }
    log_error(COMP_TEST, "Could not find single-bit state [%d], returning 0 for count enabled", state);
    return 0;
}

int gk_test_state_count_disabled(gk_repository_state state) {
    for (int i = 0; i < GK_REPOSITORY_STATE_MAX_EXP; i += 1) {
        if ((unsigned int)(1 << i) == state) {
            return g_repository_state_record_disabled[i];
        }
    }
    log_error(COMP_TEST, "Could not find single-bit state [%d], returning 0 for count disabled", state);
    return 0;
}

void gk_test_copy_source_repo_simplerepo1_dot_git() {
    if (directory_exists("./test-staging/simple-repo1.git") == 0) {
        rm_rf("./test-staging/simple-repo1.git");
    }
    copy_directory("./fixtures/simple-repo1.git", "./test-staging/simple-repo1.git");
}

void gk_test_copy_simplerepo1_from_simplerepo1_dot_gitbak() {
    gk_test_delete_simplerepo1();
    copy_directory("./fixtures/simple-repo1.gitbak", "./test-staging/simple-repo1");
    mv("./test-staging/simple-repo1/.gitbak", "./test-staging/simple-repo1/.git");
}

void gk_test_copy_empty_repository_from_empty_repository_dot_gitbak() {
    gk_test_delete_simplerepo1();
    copy_directory("./fixtures/empty-repository.gitbak", "./test-staging/empty-repository.git");
}

void gk_test_copy_simplerepo1_from_simplerepo1B_mergeconflicts_dot_gitbak() {
    gk_test_delete_simplerepo1B_mergeconflicts();
    copy_directory("./fixtures/simple-repo1-B_merge-conflicts.gitbak", "./test-staging/simple-repo1-B_merge-conflicts");
    mv("./test-staging/simple-repo1-B_merge-conflicts/.gitbak", "./test-staging/simple-repo1-B_merge-conflicts/.git");
}

void gk_test_delete_simplerepo1() {
    if (directory_exists("./test-staging/simple-repo1") == 0) {
        rm_rf("./test-staging/simple-repo1");
    }
}

void gk_test_delete_simplerepo1A() {
    if (directory_exists("./test-staging/simple-repo1-A") == 0) {
        rm_rf("./test-staging/simple-repo1-A");
    }
}

void gk_test_delete_simplerepo1B() {
    if (directory_exists("./test-staging/simple-repo1-B") == 0) {
        rm_rf("./test-staging/simple-repo1-B");
    }
}

void gk_test_delete_simplerepo1B_mergeconflicts() {
    if (directory_exists("./test-staging/simple-repo1-B_merge-conflicts") == 0) {
        rm_rf("./test-staging/simple-repo1-B_merge-conflicts");
    }
}

void gk_test_delete_simplerepo1_dot_git() {
    if (directory_exists("./test-staging/simple-repo1.git") == 0) {
        rm_rf("./test-staging/simple-repo1.git");
    }
}

void gk_test_delete_empty_repository_dot_git() {
    if (directory_exists("./test-staging/empty-repository.git") == 0) {
        rm_rf("./test-staging/empty-repository.git");
    }
}

void gk_test_delete_empty_repo_test_1() {
    if (directory_exists("./test-staging/empty-repo-test-1") == 0) {
        rm_rf("./test-staging/empty-repo-test-1");
    }
}

void gk_test_delete_empty_repo_test_2() {
    if (directory_exists("./test-staging/empty-repo-test-2") == 0) {
        rm_rf("./test-staging/empty-repo-test-2");
    }
}

int gk_test_environment_setup(void **state) {
    (void) state;
    
    if (directory_exists("test-staging") == 0) {
        rm_rf("test-staging");
    }
    create_directory("test-staging");
    return 0;
}

int gk_test_environment_teardown(void **state) {
    (void) state;
    
    return 0;
}


gk_session *gk_test_session_from_local_path(const char *repo_path) {
    gk_session *session = gk_session_new("", GK_REPOSITORY_SOURCE_URL_FILESYSTEM, repo_path, "git", &gk_test_state_change_callback, NULL);
    if (gk_session_last_result_code(session) != 0) {
        log_error(COMP_TEST, "Error creating session from path '%s': %s", repo_path, gk_session_last_result_message(session));
        gk_session_free(session);
        return NULL;
    }
    gk_session_initialize(session);
    if (gk_session_last_result_code(session) != 0) {
        log_error(COMP_TEST, "Error initializing session from path '%s': %s", repo_path, gk_session_last_result_message(session));
        gk_session_free(session);
        return NULL;
    }
    gk_test_reset_state_change_record();
    return session;
}


gk_session *gk_test_session_from_clone(const char *remote_repo, const char *local_path) {
    gk_session *session = gk_session_new(remote_repo, GK_REPOSITORY_SOURCE_URL_FILESYSTEM, local_path, "git", &gk_test_state_change_callback, NULL);
    if (gk_session_last_result_code(session) != 0) {
        log_error(COMP_TEST, "Error initializing session for clone from '%s' to path '%s': %s", remote_repo, local_path, gk_session_last_result_message(session));
        gk_session_free(session);
        return NULL;
    }

    gk_session_initialize(session);
    if (gk_session_last_result_code(session) != 0) {
        log_error(COMP_TEST, "Error initializing repository at local path '%s': %s", local_path, gk_session_last_result_message(session));
        gk_session_free(session);
        return NULL;        
    }
    
    gk_clone(session);
    if (gk_session_last_result_code(session) != 0) {
        log_error(COMP_TEST, "Error cloning repository from '%s' to local path '%s': %s", remote_repo, local_path, gk_session_last_result_message(session));
        gk_session_free(session);
        return NULL;        
    }

    if (gk_repository_state_disabled(session->repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS)) {
        log_error(COMP_TEST, "Error cloning repository from '%s' to local path '%s': local checkout does not exist after clone", remote_repo, local_path);
        gk_session_free(session);
        return NULL;        
    }

    return session;
}

void gk_test_env_conflicting_repos_a_and_b_with_extended_conflicts(gk_session **session1_ptr, gk_session **session2_ptr, void **state) {
    (void) state;
    int rc;
    
    // Clone repo to two different locations
    *session1_ptr =  gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1-A");
    gk_session *session1 = *session1_ptr;
    assert_non_null(session1);
    
    *session2_ptr = gk_test_session_from_clone("./test-staging/simple-repo1.git", "./test-staging/simple-repo1-B");
    gk_session *session2 = *session2_ptr;
    assert_non_null(session2);

    ////
    //// Modify repo-A, commit and push
    ////
    
    // modify file1 (should conflict with delete)
    rc = copy_file("fixtures/simple-repo1-modifications/file1-modified", "./test-staging/simple-repo1-A/file1");
    assert_int_equal(rc, 0);
    
    // delete file2 (should conflict should conflict with modification)
    rc = rm_file("./test-staging/simple-repo1-A/file2");
    assert_int_equal(rc, 0);
    
    // modify file 3 (should conflict with incompatible edit)
    rc = copy_file("fixtures/simple-repo1-modifications/file3-mod-incompatible-a", "./test-staging/simple-repo1-A/file3");
    assert_int_equal(rc, 0);
    
    // modify file 4 (should conflict with incompatible edit)
    rc = copy_file("fixtures/simple-repo1-modifications/file4-mod-incompatible-a", "./test-staging/simple-repo1-A/file4");
    assert_int_equal(rc, 0);
    
    // modify file 5 (should conflict with directory of same name)
    rc = copy_file("fixtures/simple-repo1-modifications/file5-mod-compatible-a", "./test-staging/simple-repo1-A/file5");
    assert_int_equal(rc, 0);

    // create binary file 6 (should conflict with new text file)
    rc = copy_file("fixtures/simple-repo1-modifications/green.png", "./test-staging/simple-repo1-A/file6");
    assert_int_equal(rc, 0);

    // create same new file 7 (should NOT conflict with identical file)
    rc = copy_file("fixtures/simple-repo1-modifications/file1-modified", "./test-staging/simple-repo1-A/file7");
    assert_int_equal(rc, 0);

    // commit and push
    gk_index_add_path(session1, "file1");
    gk_index_remove_path(session1, "file2");
    gk_index_add_path(session1, "file3");
    gk_index_add_path(session1, "file4");
    gk_index_add_path(session1, "file5");
    gk_index_add_path(session1, "file6");
    gk_index_add_path(session1, "file7");
    gk_commit(session1, "HEAD", NULL);
    assert_int_equal(gk_session_last_result_code(session1), 0);

    gk_push(session1, "origin");
    assert_int_equal(gk_session_last_result_code(session1), 0);

    ////
    //// Modify repo-B, commit
    ////
    
    // delete file1 (should conflict)
    rc = rm_file("./test-staging/simple-repo1-B/file1");
    assert_int_equal(rc, 0);
    
    // modify file2 (should conflict)
    rc = copy_file("fixtures/simple-repo1-modifications/file1-modified", "./test-staging/simple-repo1-B/file2");
    assert_int_equal(rc, 0);

    // modify file 3 in incomptabile ways (should conflict)
    rc = copy_file("fixtures/simple-repo1-modifications/file3-mod-incompatible-b", "./test-staging/simple-repo1-B/file3");
    assert_int_equal(rc, 0);

    // modify file 4 in incompatible ways (should conflict)
    rc = copy_file("fixtures/simple-repo1-modifications/file4-mod-incompatible-b", "./test-staging/simple-repo1-B/file4");
    assert_int_equal(rc, 0);

    // Delete file5 and create a directory in its place (should conflict)
    rc = rm_file("./test-staging/simple-repo1-B/file5");
    assert_int_equal(rc, 0);
    rc = create_directory("./test-staging/simple-repo1-B/file5");    
    assert_int_equal(rc, 0);
    rc = copy_file("fixtures/simple-repo1-modifications/file1-modified", "./test-staging/simple-repo1-B/file5/child1");
    
    // create text file 6 (should conflict)
    rc = copy_file("fixtures/simple-repo1-modifications/file1-modified", "./test-staging/simple-repo1-B/file6");
    assert_int_equal(rc, 0);

    // create same new file 7 (should NOT conflict)
    rc = copy_file("fixtures/simple-repo1-modifications/file1-modified", "./test-staging/simple-repo1-A/file7");
    assert_int_equal(rc, 0);

    // commit but don't push
    gk_index_remove_path(session2, "file1");
    gk_index_add_path(session2, "file2");
    gk_index_add_path(session2, "file3");
    gk_index_add_path(session2, "file4");
    gk_index_add_path(session2, "file5/child1");
    gk_index_add_path(session2, "file6");
    gk_index_add_path(session2, "file7");
    gk_commit(session2, "HEAD", NULL);
    assert_int_equal(gk_session_last_result_code(session2), 0);// commit and push

    ////
    //// Repo B fetch
    ////

    // Repo-B fetch
    gk_fetch(session2, "origin");
    assert_int_equal(gk_session_last_result_code(session2), 0);

    // there should now be conflicts in repo B upon merging
}
