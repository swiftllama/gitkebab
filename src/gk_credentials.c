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

    memset(credential, 0, sizeof(gk_session_credential_t));
    credential->credential_type = CREDENTIAL_SSH_KEY_MEMORY;
    credential->ssh_private_key_bytes = private_key_bytes == NULL ? NULL : strdup(private_key_bytes);
    credential->ssh_public_key_bytes = public_key_bytes == NULL ? NULL : strdup(public_key_bytes);
    credential->ssh_private_key_passphrase = private_key_passphrase == NULL ? NULL : strdup(private_key_passphrase);

    return gk_result_success();
}

gk_result_t *gk_session_credential_ssh_key_file_init(gk_session_credential_t *credential, const char *private_key_path, const char *public_key_path, const char *private_key_passphrase) {
    gk_result_t *result = NULL;

    if (credential == NULL) {
        result = gk_result(-1, "Cannot initialize ssh key credential (file), credential is NULL");
        log_error(COMP_AUTH, gk_result_message(result));
        return result;
    }

    memset(credential, 0, sizeof(gk_session_credential_t));
    credential->credential_type = CREDENTIAL_SSH_KEY_FILE;
    credential->ssh_private_key_path = private_key_path == NULL ? NULL : strdup(private_key_path);
    credential->ssh_public_key_path = public_key_path == NULL ? NULL : strdup(public_key_path);
    credential->ssh_private_key_passphrase = private_key_passphrase == NULL ? NULL : strdup(private_key_passphrase);
    
    return gk_result_success();
}

gk_result_t *gk_session_credential_username_password_init(gk_session_credential_t *credential, const char *username, const char *password) {
    gk_result_t *result = NULL;
    
    if (credential == NULL) {
        result = gk_result(-1, "Cannot initialize username/password, credential is NULL");
        log_error(COMP_AUTH, gk_result_message(result));
        return result;
    }

    memset(credential, 0, sizeof(gk_session_credential_t));
    credential->credential_type = CREDENTIAL_USERNAME_PASSWORD;
    credential->username = username == NULL ? NULL : strdup(username);
    credential->password = password == NULL ? NULL : strdup(password);

    return gk_result_success();
}

void gk_session_credential_free_members(gk_session_credential_t *credential) {
    if (credential == NULL) {
        return;
    }

    log_error(COMP_GENERAL, "DBG A1");
    if (credential->ssh_private_key_bytes != NULL) {
        log_error(COMP_GENERAL, "DBG A2");
        free((void *)credential->ssh_private_key_bytes);
    }
    log_error(COMP_GENERAL, "DBG A3");
    if (credential->ssh_public_key_bytes != NULL) {
        log_error(COMP_GENERAL, "DBG A4");
        free((void *)credential->ssh_public_key_bytes);
    }
    log_error(COMP_GENERAL, "DBG A5");
    if (credential->ssh_private_key_path != NULL) {
        log_error(COMP_GENERAL, "DBG A6");
        free((void *)credential->ssh_private_key_path);
    }
    log_error(COMP_GENERAL, "DBG A7");
    if (credential->ssh_public_key_path != NULL) {
        log_error(COMP_GENERAL, "DBG A8");
        free((void *)credential->ssh_public_key_path);
    }
    log_error(COMP_GENERAL, "DBG A9");
    if (credential->ssh_private_key_passphrase != NULL) {
        log_error(COMP_GENERAL, "DBG A10");
        free((void *)credential->ssh_private_key_passphrase);
    }
    log_error(COMP_GENERAL, "DBG A11");
    if (credential->username != NULL) {
        log_error(COMP_GENERAL, "DBG A12");
        free((void *)credential->username);
    }
    log_error(COMP_GENERAL, "DBG A13");
    if (credential->password != NULL) {
        log_error(COMP_GENERAL, "DBG A14");
        free((void *)credential->password);
    }
}
