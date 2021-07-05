#include "gk_credentials.h"
#include "gk_results.h"
#include "gk_session.h"

#include "gk_logging.h"
#include <string.h>


gk_result_t *gk_session_credential_ssh_key_memory_init(gk_session_credential_t *credential, const char *private_key_bytes, const char *public_key_bytes, const char *private_key_passphrase) {
    if (credential == NULL) {
        return gk_fail_result(&COMP_AUTH, -1, "Cannot initialize ssh key credential (memory), credential is NULL");
    }

    memset(credential, 0, sizeof(gk_session_credential_t));
    credential->credential_type = CREDENTIAL_SSH_KEY_MEMORY;
    credential->ssh_private_key_bytes = private_key_bytes == NULL ? NULL : strdup(private_key_bytes);
    credential->ssh_public_key_bytes = public_key_bytes == NULL ? NULL : strdup(public_key_bytes);
    credential->ssh_private_key_passphrase = private_key_passphrase == NULL ? NULL : strdup(private_key_passphrase);

    return gk_result_success();
}

gk_result_t *gk_session_credential_ssh_key_file_init(gk_session_credential_t *credential, const char *private_key_path, const char *public_key_path, const char *private_key_passphrase) {
    if (credential == NULL) {
        return gk_fail_result(&COMP_AUTH, -1, "Cannot initialize ssh key credential (file), credential is NULL");
    }

    memset(credential, 0, sizeof(gk_session_credential_t));
    credential->credential_type = CREDENTIAL_SSH_KEY_FILE;
    credential->ssh_private_key_path = private_key_path == NULL ? NULL : strdup(private_key_path);
    credential->ssh_public_key_path = public_key_path == NULL ? NULL : strdup(public_key_path);
    credential->ssh_private_key_passphrase = private_key_passphrase == NULL ? NULL : strdup(private_key_passphrase);
    
    return gk_result_success();
}

gk_result_t *gk_session_credential_username_password_init(gk_session_credential_t *credential, const char *username, const char *password) {
    if (credential == NULL) {
        return gk_fail_result(&COMP_AUTH, -1, "Cannot initialize username/password, credential is NULL");
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

    free((void *)credential->ssh_private_key_bytes);
    free((void *)credential->ssh_public_key_bytes);
    free((void *)credential->ssh_private_key_path);
    free((void *)credential->ssh_public_key_path);
    free((void *)credential->ssh_private_key_passphrase);
    free((void *)credential->username);
    free((void *)credential->password);
}
