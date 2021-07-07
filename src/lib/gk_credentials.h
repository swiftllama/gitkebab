
#ifndef __GITKEBAB_CREDENTIALS_H__
#define __GITKEBAB_CREDENTIALS_H__

#include "gk_results.h"

typedef enum {
    CREDENTIAL_SSH_KEY_MEMORY,
    CREDENTIAL_SSH_KEY_FILE,
    CREDENTIAL_USERNAME_PASSWORD
} gk_session_credential_type;
    
typedef struct {
    gk_session_credential_type credential_type;
    const char *ssh_private_key_bytes;
    const char *ssh_public_key_bytes;
    const char *ssh_private_key_path;
    const char *ssh_public_key_path;
    const char *ssh_private_key_passphrase;
    const char *username;
    const char *password;
} gk_session_credential;

gk_result *gk_session_credential_ssh_key_memory_init(gk_session_credential *credential, const char *private_key_bytes, const char *public_key_bytes, const char *private_key_passphrase);

gk_result *gk_session_credential_ssh_key_file_init(gk_session_credential *credential, const char *private_key_path, const char *public_key_path, const char *private_key_passphrase);

gk_result *gk_session_credential_username_password_init(gk_session_credential *credential, const char *username, const char *password);

void gk_session_credential_free_members(gk_session_credential *credential);

#endif // __GITKEBAB_CREDENTIALS_H__
