
#ifndef __GITKEBAB_COMMIT_H__
#define __GITKEBAB_COMMIT_H__

#include "gk_repository.h"

#define GK_OBJECT_ID_STR_LENGTH 40
typedef struct gk_object_id {  char id[GK_OBJECT_ID_STR_LENGTH+1]; } gk_object_id;

size_t gk_count_reflog_entries(gk_session *session, const char* ref_name);
int gk_commit(gk_session *session, const char* commit_message, gk_object_id *out_commit_id);
int gk_resolve_reference(gk_session *session, const char *ref_name, gk_object_id *object_id);

const char * gk_object_id_ptr(gk_object_id *object_id);

#endif // __GITKEBAB_COMMIT_H__
