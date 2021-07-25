
#ifndef __GK_REMOTES_H__
#define __GK_REMOTES_H__

#include "gk_repository.h"

int gk_repository_fetch(gk_repository *repository, gk_session_credential *credential, const char *remote_name);
int gk_repository_push(gk_repository *repository, gk_session_credential *credential, const char *remote_name);
int gk_repository_clone(gk_repository *repository, gk_session_credential *credential);

#endif // __GK_REMOTES_H__
