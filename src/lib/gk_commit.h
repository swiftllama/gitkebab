
#ifndef __GITKEBAB_COMMIT_H__
#define __GITKEBAB_COMMIT_H__

#include "gk_session.h"

size_t gk_session_count_reflog_entries(gk_session_t *session, const char* ref_name);
int gk_session_commit(gk_session_t *session, const char *ref_name, const char* commit_message);

#endif // __GITKEBAB_COMMIT_H__
