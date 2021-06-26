#include "gk_credentials.h"
#include "gk_results.h"
#include "gk_session.h"

#include "gk_logging.h"
#include <string.h>


gk_result_t *gk_session_credential_ssh_key_memory_init(gk_session_credential_t *credential, const char *private_key_bytes, const char *public_key_bytes, const char *private_key_passphrase) {
    gk_result_t *result = NULL;
    
    if (credential == NULL) {
        result = gk_result(-1, "Cannot initialize ssh key credential (memory), credential is NULL");
        log_error(COMP_AUTH, gk_result_message(result));
        return result;
    }

    credential->credential_type = CREDENTIAL_SSH_KEY_MEMORY;
    credential->ssh_private_key_bytes = strdup(private_key_bytes);
    credential->ssh_public_key_bytes = strdup(public_key_bytes);
    credential->ssh_private_key_passphrase = strdup(private_key_passphrase);

    return gk_result_success();
}

gk_result_t *gk_session_credential_ssh_key_file_init(gk_session_credential_t *credential, const char *private_key_path, const char *public_key_path, const char *private_key_passphrase) {
    gk_result_t *result = NULL;

    if (credential == NULL) {
        result = gk_result(-1, "Cannot initialize ssh key credential (file), credential is NULL");
        log_error(COMP_AUTH, gk_result_message(result));
        return result;
    }

    credential->credential_type = CREDENTIAL_SSH_KEY_FILE;
    credential->ssh_private_key_path = strdup(private_key_path);
    credential->ssh_public_key_path = strdup(public_key_path);
    credential->ssh_private_key_passphrase = strdup(private_key_passphrase);
    
    return gk_result_success();
}

gk_result_t *gk_session_credential_username_password_init(gk_session_credential_t *credential, const char *username, const char *password) {
    gk_result_t *result = NULL;
    
    if (credential == NULL) {
        result = gk_result(-1, "Cannot initialize username/password, credential is NULL");
        log_error(COMP_AUTH, gk_result_message(result));
        return result;
    }

    credential->credential_type = CREDENTIAL_USERNAME_PASSWORD;
    credential->username = strdup(username);
    credential->password = strdup(password);

    return gk_result_success();
}

void gk_session_credential_free_members(gk_session_credential_t *credential) {
    if (credential == NULL) {
        return;
    }

    if (credential->ssh_private_key_bytes != NULL) {
        free((void *)credential->ssh_private_key_bytes);
    }
    if (credential->ssh_public_key_bytes != NULL) {
        free((void *)credential->ssh_public_key_bytes);
    }
    if (credential->ssh_private_key_path != NULL) {
        free((void *)credential->ssh_private_key_path);
    }
    if (credential->ssh_public_key_path != NULL) {
        free((void *)credential->ssh_public_key_path);
    }
    if (credential->ssh_private_key_passphrase != NULL) {
        free((void *)credential->ssh_private_key_passphrase);
    }
    if (credential->username != NULL) {
        free((void *)credential->username);
    }
    if (credential->password != NULL) {
        free((void *)credential->password);
    }
}
