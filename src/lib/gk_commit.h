
#ifndef __GITKEBAB_COMMIT_H__
#define __GITKEBAB_COMMIT_H__

#include "gk_session.h"

#define GK_OBJECT_ID_LENGTH 20
typedef struct gk_object_id_t { unsigned char id[GK_OBJECT_ID_LENGTH]; } gk_object_id_t;

size_t gk_session_count_reflog_entries(gk_session_t *session, const char* ref_name);
int gk_session_commit(gk_session_t *session, const char *ref_name, const char* commit_message, gk_object_id_t *out_commit_id);
int gk_session_resolve_reference(gk_session_t *session, const char *ref_name, const char* commit_message);

char *gk_object_id_hex_string_new(gk_object_id_t *object_id);

#endif // __GITKEBAB_COMMIT_H__
