
#include "gitkebab.h"
#include "stdio.h"
#include <string.h>

void session_state_changed(const char *session_id, gk_repository *repository, gk_session_progress *progress) {
    (void) repository;
    const char *sess_id = session_id == NULL ? "(unknown sess id)" : session_id;
    if (progress != NULL) {
        printf("[SESSION PROGRESS] (session %s) %s (TOTAL: %d%%)\n", sess_id, progress->description, progress->percent);
    }
    else {
        printf("[SESSION PROGRESS] (session %s) ...\n", sess_id);
    }
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    
    COMP_INIT.level = LOG_INFO;

    if (argc < 4) {
        printf("Usage: example1 URL_REPO DEST_PATH KEY_PATH [KEY_PASSPHRASE]\n");
        return 1;
    }
    
    gk_init(NULL, LOG_INFO);
    //gk_libgit2_set_log_level(LOG_DEBUG);

    const char *repo_url = argv[1];
    const char *dest_path = argv[2];
    const char *key_path = argv[3];
    const char *key_passphrase = argc <= 4 ? NULL : argv[4];

    gk_repository_source_url_type url_type = strncmp("ssh", repo_url, 3) == 0 ? GK_REPOSITORY_SOURCE_URL_SSH : GK_REPOSITORY_SOURCE_URL_HTTP;
    gk_session *session = gk_session_new(repo_url, url_type, dest_path, "git", &session_state_changed, NULL);

    gk_session_initialize(session);
    if (gk_session_last_result_code(session) != GK_SUCCESS) {
        printf("Error #%d occurred while initializing session: %s\n", gk_session_last_result_code(session), gk_session_last_result_message(session));
    }
    gk_session_credential_ssh_key_file_init(session, "git", key_path, NULL, key_passphrase);
    gk_clone(session);
    
    if (gk_session_last_result_code(session) == GK_SUCCESS) {
        printf("cloned successfully!\n");
    }
    else {
        printf("Error #%d occurred in session while cloning: %s\n", gk_session_last_result_code(session), gk_session_last_result_message(session));
    }

    gk_session_free(session);
    return 0;
}
