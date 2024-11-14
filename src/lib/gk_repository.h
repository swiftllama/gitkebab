

#ifndef __GITKEBAB_SESSION_H__
#define __GITKEBAB_SESSION_H__

#include <stdlib.h>

#include "gk_results.h"
#include "gk_credentials.h"
#include "gk_session_progress.h"
#include "gk_logging.h"
#include "gk_types.h"

gk_repository *gk_repository_new();
void gk_repository_init(gk_repository *repository, const char *source_url, gk_repository_source_url_type source_url_type, const char *local_path, const char *user);
void gk_repository_update_remote(gk_repository *repository, const char* remote_url);
int gk_open_local_repository(gk_session *session);
int gk_create_local_repository(gk_session *session, const char *main_branch_name);
void gk_repository_free(gk_repository *repository);

int gk_repository_state_enabled(gk_repository *repository, unsigned int states);
int gk_repository_state_disabled(gk_repository *repository, unsigned int states);
void gk_repository_state_set(gk_repository *repository, int states_enable);
void gk_repository_state_unset(gk_repository *repository, int states_disable);

int gk_prepend_repository_path(gk_session *session, char *buffer, size_t buffer_length, const char *path);
void gk_repository_print_state(gk_repository *repository);
#endif // __GITKEBAB_SESSION_H__
