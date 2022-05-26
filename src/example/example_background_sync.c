
#include "gitkebab.h"
#include "stdio.h"

#if defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#else
#include <unistd.h>
#endif


// NOTE: private key must include newlines!
char *judo_key = ""
"-----BEGIN RSA PRIVATE KEY-----\n"
"...\n"
"-----END RSA PRIVATE KEY-----";

char *judo_key_pub = NULL;


void sleepSeconds(int seconds)
{
#if defined(_WIN32) || defined(__WIN32__)
    Sleep(seconds*1000);
#else
    usleep(seconds * 1000 * 1000);
#endif
}



void session_state_changed(const char *session_id, gk_repository *repository, gk_session_progress *progress) {
    (void) repository;
    const char *sess_id = session_id == NULL ? "(unknown sess id)" : session_id;
    if (progress != NULL)
        printf("[SESSION PROGRESS] (session %s) %s (TOTAL: %d%%)\n", sess_id, progress->description, progress->percent);
    else
        printf("[SESSION PROGRESS] (session %s) ...\n", sess_id);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    
    COMP_INIT.level = LOG_TRACE;
    
    gk_init(NULL, LOG_DEBUG);
    gk_session *session = gk_session_new("git@gitea.ptskl.com:volund/experimental-notebook.git", GK_REPOSITORY_SOURCE_URL_SSH, "/tmp/clone1", "git", &session_state_changed, NULL);

    gk_session_initialize(session
                          );
    if (gk_session_last_result_code(session) != GK_SUCCESS) {
        printf("Error #%d occurred while initializing session: %s\n", gk_session_last_result_code(session), gk_session_last_result_message(session));
    }
    gk_session_credential_ssh_key_memory_init(session, "git", judo_key, judo_key_pub, NULL);
    gk_background_sync(session);

    int i = 0;
    while (gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_BACKGROUND_SYNC_IN_PROGRESS) == 1) {
        sleepSeconds(1);
        printf(".");
        if (i >= 80) {
            printf("\n");
            i = 0;
        }
        i += 1;
    }
    
    if (gk_session_last_result_code(session) == GK_SUCCESS) {
        printf("cloned successfully!\n");
    }
    else {
        printf("Error #%d occurred in session while cloning: %s\n", gk_session_last_result_code(session), gk_session_last_result_message(session));
    }

    gk_session_free(session);
    return 0;
}
