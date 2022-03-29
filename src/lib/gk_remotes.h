
#ifndef __GK_REMOTES_H__
#define __GK_REMOTES_H__

#include "gk_repository.h"

int gk_clone(gk_session *session);
int gk_fetch(gk_session *session, const char *remote_name);
int gk_push(gk_session *session, const char *remote_name);

gk_remote_list *gk_remotes_list(gk_session *session);
void gk_remote_list_free(gk_remote_list *list);
gk_remote *gk_remote_new(const char *name, const char *url);
void gk_remote_free(gk_remote *remote);
    
int gk_remote_create(gk_session *session, const char* name, const char* url);
int gk_remote_delete(gk_session *session, const char* name);

#endif // __GK_REMOTES_H__
