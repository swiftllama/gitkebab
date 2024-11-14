
#ifndef __GK_INDEX_H__
#define __GK_INDEX_H__

#include "gk_repository.h"

int gk_index_add_path(gk_session *session, const char *path);
int gk_index_remove_path(gk_session *session, const char *path);
int gk_index_add_all(gk_session *session, const char* pattern);
int gk_index_update_all(gk_session *session, const char* pattern);


#endif // __GK_INDEX_H__
