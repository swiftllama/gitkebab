
#ifndef __GK_INDEX_H__
#define __GK_INDEX_H__

#include "gk_session.h"

int gk_session_add_path_to_index(gk_session_t *session, const char *path);
int gk_session_remove_path_from_index(gk_session_t *session, const char *path);

#endif // __GK_INDEX_H__
