
#ifndef __GK_SESSION_H__
#define __GK_SESSION_H__

#include "gk_types.h"

void gk_session_init(gk_session *session, gk_repository *repository, gk_repository_credential *credential);
int gk_session_context_sanity_check(gk_session *session, log_Component *component, const char *purpose);

#endif __GK_SESSION_H__
