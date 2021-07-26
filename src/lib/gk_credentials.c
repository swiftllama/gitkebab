#include "gk_credentials.h"
#include "gk_results.h"
#include "gk_repository.h"

#include "gk_logging.h"
#include <string.h>


void gk_session_credential_init(gk_session *session) {
    memset((void *)&session->credential, 0, sizeof(gk_session_credential));
}

int gk_session_credential_ssh_key_memory_init(gk_session *session, const char *private_key_bytes, const char *public_key_bytes, const char *private_key_passphrase) {
    if (session == NULL) {
        log_error(COMP_AUTH, "Cannot initialize ssh key (memory), session is NULL");
        return GK_FAILURE;
    }

    gk_session_credential_free_members(&session->credential);
    gk_session_credential_init(session);
    session->credential.credential_type = CREDENTIAL_SSH_KEY_MEMORY;
    session->credential.ssh_private_key_bytes = private_key_bytes == NULL ? NULL : strdup(private_key_bytes);
    session->credential.ssh_public_key_bytes = public_key_bytes == NULL ? NULL : strdup(public_key_bytes);
    session->credential.ssh_private_key_passphrase = private_key_passphrase == NULL ? NULL : strdup(private_key_passphrase);

    return GK_SUCCESS;
}

int gk_session_credential_ssh_key_file_init(gk_session *session, const char *private_key_path, const char *public_key_path, const char *private_key_passphrase) {
    if (session == NULL) {
        log_error(COMP_AUTH, "Cannot initialize ssh key (file), session is NULL");
        return GK_FAILURE;
    }

    gk_session_credential_free_members(&session->credential);
    gk_session_credential_init(session);
    session->credential.credential_type = CREDENTIAL_SSH_KEY_FILE;
    session->credential.ssh_private_key_path = private_key_path == NULL ? NULL : strdup(private_key_path);
    session->credential.ssh_public_key_path = public_key_path == NULL ? NULL : strdup(public_key_path);
    session->credential.ssh_private_key_passphrase = private_key_passphrase == NULL ? NULL : strdup(private_key_passphrase);
    
    return GK_SUCCESS;
}

int gk_session_credential_username_password_init(gk_session *session, const char *username, const char *password) {
    if (session == NULL) {
        log_error(COMP_AUTH, "Cannot initialize username/password credential, session is NULL");
        return GK_FAILURE;
    }    

    gk_session_credential_free_members(&session->credential);
    gk_session_credential_init(session);
    session->credential.credential_type = CREDENTIAL_USERNAME_PASSWORD;
    session->credential.username = username == NULL ? NULL : strdup(username);
    session->credential.password = password == NULL ? NULL : strdup(password);

    return GK_FAILURE;
}

void gk_session_credential_free_members(gk_session_credential *credential) {
    if (credential == NULL) {
        return;
    }

    free((void *)credential->ssh_private_key_bytes);
    free((void *)credential->ssh_public_key_bytes);
    free((void *)credential->ssh_private_key_path);
    free((void *)credential->ssh_public_key_path);
    free((void *)credential->ssh_private_key_passphrase);
    free((void *)credential->username);
    free((void *)credential->password);
    credential->ssh_private_key_bytes = NULL;
    credential->ssh_public_key_bytes = NULL;
    credential->ssh_private_key_path = NULL;
    credential->ssh_public_key_path = NULL;
    credential->ssh_private_key_passphrase = NULL;
    credential->username = NULL;
    credential->password = NULL;
}
