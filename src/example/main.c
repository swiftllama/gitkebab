
#include "gitkebab.h"
#include "stdio.h"

// NOTE: private key must include newlines!
char *judo_key = ""
"-----BEGIN RSA PRIVATE KEY-----\n"
"...\n"
"-----END RSA PRIVATE KEY-----";

//char *judo_key_pub="ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAACAQCzb3WvCWm2DsGG+jGQzJ3wuDD3muYDeorRvhoRGC+lMY2gCawIQ5/PboGexC1Ulffs81kbpbYXoTUA/kFMjyMmHKGxQ6KzXQqYQBZw1294NqjbuuZvgc6nWOOjnK7VQ9guor0HThnsas9j0963vp+Rvbqp42dFoqPxUGakegMb9Qn6fX3u7T5OkRbg5Scg7LVRdCAYC3IijFqlp9AWvybnU0q80cDuMKtJcIl34DlzR/D/Pz4gzjvtIgeFqjl87hCDnWdltOmFOKeYAqcsdQOMgx2yA4GVKdBC0aQ4fPRF3N7XzuAGOpezHnuDZtW161LAuq4a5LzGmB3tKO3qbznOfCulnrd9uSv9DXtoLku88Z9R8Pw8KLNe2d1R64RWaOcUbmmYF5S0J0jGHzbPXfldZUa1vZly3GRijx6GRSkWVP5FUzxQ50x+LqtSauSOOmQ90da2Vuim0hBafSVJRqFnGlEb2+gfbXFh6rO6nH6QFecqJWMQjPWN28SGCw8xC1gj+Afg7ac+WoTp+oGH5qB78wRlhvuRDeYvezlGWnqf58T4aokNlS1+h5IsWHqtgzp6h/mHlvQ69EjvfWwpd2rKPEsoxl2QbyArfjNU6JYrJykXUP1jpS9CvXt0UdPu+xcooU1UgQukMgFJ7/UPQtNC2PLZ5N9OR4TSGGjz0zulaw== volund@gmail.com";
char *judo_key_pub = NULL;


void session_progress(const char *session_id, gk_session_progress *progress) {
    printf("[SESSION PROGRESS] (session %s) %s (TOTAL: %d%%)\n", session_id, progress->description, progress->percent);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    
    COMP_INIT.level = LOG_TRACE;
    
    gk_init();
    //gk_libgit2_set_log_level(LOG_DEBUG);

    gk_session *session = gk_session_new("git@gitea.ptskl.com:volund/experimental-notebook.git", GK_REPOSITORY_SOURCE_URL_SSH, "/tmp/clone1", "git", &session_progress, NULL, NULL);

    gk_session_initialize(session);
    if (gk_session_last_result_code(session) != GK_SUCCESS) {
        printf("Error #%d occurred while initializing session: %s\n", gk_session_last_result_code(session), gk_session_last_result_message(session));
    }
    gk_session_credential_ssh_key_memory_init(session, "git", judo_key, judo_key_pub, NULL);
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
