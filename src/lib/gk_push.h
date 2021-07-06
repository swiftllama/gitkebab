
#ifndef __GK_PUSH_H__
#define __GK_PUSH_H__

#include "gk_session.h"

int gk_session_fetch(gk_session_t *session, gk_session_credential_t *credential, const char *remote_name);
int gk_session_push(gk_session_t *session, gk_session_credential_t *credential, const char *remote_name);
                    
#endif // __GK_PUSH_H__
