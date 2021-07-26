
#ifndef __GK_REMOTES_H__
#define __GK_REMOTES_H__

#include "gk_repository.h"

int gk_clone(gk_session *session);
int gk_fetch(gk_session *session, const char *remote_name);
int gk_push(gk_session *session, const char *remote_name);

#endif // __GK_REMOTES_H__
