#include "gk_credentials.h"
#include "gk_results.h"
#include "gk_repository.h"

#include "gk_logging.h"
#include <string.h>

static char credential_description[256];

void gk_session_credential_init(gk_session *session) {
    memset((void *)&session->credential, 0, sizeof(gk_session_credential));
}

int gk_session_credential_ssh_key_memory_init(gk_session *session, const char* username, const char *private_key_bytes, const char *public_key_bytes, const char *private_key_passphrase) {
    if (session == NULL) {
        log_error(COMP_AUTH, "Cannot initialize ssh key (memory), session is NULL");
        return GK_FAILURE;
    }

    gk_session_credential_free_members(&session->credential);
    gk_session_credential_init(session);
    session->credential.credential_type = CREDENTIAL_SSH_KEY_MEMORY;
    session->credential.username = username == NULL ? NULL : strdup(username);
    session->credential.ssh_private_key_bytes = private_key_bytes == NULL ? NULL : strdup(private_key_bytes);
    session->credential.ssh_public_key_bytes = public_key_bytes == NULL ? NULL : strdup(public_key_bytes);
    session->credential.ssh_private_key_passphrase = private_key_passphrase == NULL ? NULL : strdup(private_key_passphrase);

    return GK_SUCCESS;
}

int gk_session_credential_ssh_key_file_init(gk_session *session, const char* username, const char *private_key_path, const char *public_key_path, const char *private_key_passphrase) {
    if (session == NULL) {
        log_error(COMP_AUTH, "Cannot initialize ssh key (file), session is NULL");
        return GK_FAILURE;
    }

    gk_session_credential_free_members(&session->credential);
    gk_session_credential_init(session);
    session->credential.credential_type = CREDENTIAL_SSH_KEY_FILE;
    session->credential.username = username == NULL ? NULL : strdup(username);
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

    return GK_SUCCESS;
}

int gk_session_free_credential(gk_session *session) {
    if (session == NULL) {
        log_error(COMP_AUTH, "Cannot free session credential, session is NULL");
        return GK_FAILURE;
    }
    gk_session_credential_free_members(&session->credential);
    return GK_SUCCESS;
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


const char *gk_credential_description(gk_session_credential *credential) {
    if (credential == NULL) return "(null)";
    const char *str_type = "(unknown type)";
    if (credential->credential_type == CREDENTIAL_SSH_KEY_FILE) {
        str_type = "SSH key file";
    }
    else if (credential->credential_type == CREDENTIAL_SSH_KEY_MEMORY) {
        str_type = "SSH KEY data (in memory)";
    }
    else if (credential->credential_type == CREDENTIAL_USERNAME_PASSWORD) {
        str_type = "Username password";
    }

    int have_ssh_private_key_bytes = credential->ssh_private_key_bytes != NULL;
    int have_ssh_public_key_bytes = credential->ssh_public_key_bytes != NULL;
    int have_ssh_private_key_path = credential->ssh_private_key_path != NULL;
    int have_ssh_public_key_path = credential->ssh_public_key_path != NULL;
    int have_ssh_private_key_passphrase = credential->ssh_private_key_passphrase != NULL;
    int have_username = credential->username != NULL;
    int have_password = credential->password != NULL;

    snprintf(credential_description, 255, "%s [with private_key_bytes:%d, public_key_bytes:%d, private_key_path:%d, public_key_path:%d, private_key_passphrase:%d, username:%d, password:%d",
             str_type,
             have_ssh_private_key_bytes,
             have_ssh_public_key_bytes,
             have_ssh_private_key_path,
             have_ssh_public_key_path,
             have_ssh_private_key_passphrase,
             have_username,
             have_password);
    return credential_description;
}
