
#include "gk_test_env_utils.h"
#include "gk_test_filesystem_utils.h"
#include "gitkebab.h"

gk_session_credential g_empty_credential;

void gk_test_copy_source_repo_simplerepo1_dot_git() {
    if (directory_exists("test-staging/simple-repo1.git") == 0) {
        rm_rf("test-staging/simple-repo1.git");
    }
    copy_directory("./src/test/fixtures/simple-repo1.git", "test-staging/simple-repo1.git");
}

void gk_test_copy_simplerepo1_from_simplerepo1_dot_gitbak() {
    gk_test_delete_simplerepo1();
    copy_directory("./src/test/fixtures/simple-repo1.gitbak", "test-staging/simple-repo1");
    mv("test-staging/simple-repo1/.gitbak", "test-staging/simple-repo1/.git");
}

void gk_test_copy_simplerepo1_from_simplerepo1B_mergeconflicts_dot_gitbak() {
    gk_test_delete_simplerepo1B_mergeconflicts();
    copy_directory("./src/test/fixtures/simple-repo1-B_merge-conflicts.gitbak", "test-staging/simple-repo1-B_merge-conflicts");
    mv("test-staging/simple-repo1-B_merge-conflicts/.gitbak", "test-staging/simple-repo1-B_merge-conflicts/.git");
}

void gk_test_delete_simplerepo1() {
    if (directory_exists("test-staging/simple-repo1") == 0) {
        rm_rf("test-staging/simple-repo1");
    }
}

void gk_test_delete_simplerepo1A() {
    if (directory_exists("test-staging/simple-repo1-A") == 0) {
        rm_rf("test-staging/simple-repo1-A");
    }
}

void gk_test_delete_simplerepo1B() {
    if (directory_exists("test-staging/simple-repo1-B") == 0) {
        rm_rf("test-staging/simple-repo1-B");
    }
}

void gk_test_delete_simplerepo1B_mergeconflicts() {
    if (directory_exists("test-staging/simple-repo1-B_merge-conflicts") == 0) {
        rm_rf("test-staging/simple-repo1-B_merge-conflicts");
    }
}

void gk_test_delete_simplerepo1_dot_git() {
    if (directory_exists("test-staging/simple-repo1.git") == 0) {
        rm_rf("test-staging/simple-repo1.git");
    }
}

int gk_test_environment_setup(void **state) {
    (void) state;
    
    if (directory_exists("test-staging") == 0) {
        rm_rf("test-staging");
    }
    create_directory("test-staging");
    gk_session_credential_username_password_init(&g_empty_credential, "", "");
    return 0;
}

int gk_test_environment_teardown(void **state) {
    (void) state;
    
    gk_session_credential_free_members(&g_empty_credential);
    return 0;
}


void gk_test_session_progress_verbose(gk_session_progress *progress) {
    if (progress != NULL) {
        log_info(COMP_TEST, "PROGRESS [%s] (%zu%%)", progress->description, progress->percent);
    }
    else {
        log_error(COMP_TEST, "Error in progress callback, callback invoked with NULL progress struct");
    }
}

void gk_test_session_progress_null(gk_session_progress *progress) {
    (void) progress;
}



gk_session *gk_test_session_from_local_path(const char *repo_path) {
    gk_session *session = gk_session_new();
    gk_session_init(session, "", repo_path, "git", &gk_test_session_progress_verbose, NULL);
    if (gk_result_code(session->last_result) != 0) {
        log_error(COMP_TEST, "Error initializing session from path '%s': %s", repo_path, gk_result_message(session->last_result));
        gk_session_free(session);
        return NULL;
    }
    gk_session_open_local_repository(session);
    if (gk_result_code(session->last_result) != 0) {
        log_error(COMP_TEST, "Error opening local repository in path '%s': %s", repo_path, gk_result_message(session->last_result));
        gk_session_free(session);
        return NULL;
    }
    return session;
}


gk_session *gk_test_session_from_clone(const char *remote_repo, const char *local_path) {
    gk_session *session = gk_session_new();
    gk_session_init(session, remote_repo, local_path, "git", &gk_test_session_progress_verbose, NULL);
    if (gk_result_code(session->last_result) != 0) {
        log_error(COMP_TEST, "Error initializing session for clone from '%s' to path '%s': %s", remote_repo, local_path, gk_result_message(session->last_result));
        gk_session_free(session);
        return NULL;
    }

    gk_session_clone(session, &g_empty_credential);
    if (gk_result_code(session->last_result) != 0) {
        log_error(COMP_TEST, "Error cloning repository from '%s' to local path '%s': %s", remote_repo, local_path, gk_result_message(session->last_result));
        gk_session_free(session);
        return NULL;        
    }

    if (gk_session_state_disabled(session, GK_SESSION_STATE_LOCAL_CHECKOUT_EXISTS)) {
        log_error(COMP_TEST, "Error cloning repository from '%s' to local path '%s': local checkout does not exist after clone", remote_repo, local_path);
        gk_session_free(session);
        return NULL;        
    }

    return session;
}
