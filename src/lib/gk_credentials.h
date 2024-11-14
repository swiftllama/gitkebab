
#ifndef __GITKEBAB_CREDENTIALS_H__
#define __GITKEBAB_CREDENTIALS_H__

#include "gk_results.h"
#include "gk_types.h"

int gk_session_credential_ssh_key_memory_init(gk_session *session, const char* username, const char *private_key_bytes, const char *public_key_bytes, const char *private_key_passphrase);
int gk_session_credential_ssh_key_file_init(gk_session *session, const char* username, const char *private_key_path, const char *public_key_path, const char *private_key_passphrase);
int gk_session_credential_username_password_init(gk_session *session, const char *username, const char *password);
void gk_session_credential_free_members(gk_session_credential *credential);
int gk_session_free_credential(gk_session *session);
void gk_session_credential_init(gk_session *session);
const char *gk_credential_description(gk_session_credential *credential);

#endif // __GITKEBAB_CREDENTIALS_H__

