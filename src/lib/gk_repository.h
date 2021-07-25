

#ifndef __GITKEBAB_SESSION_H__
#define __GITKEBAB_SESSION_H__

#include <stdlib.h>

#include "gk_results.h"
#include "gk_credentials.h"
#include "gk_session_progress.h"
#include "gk_logging.h"
#include "gk_types.h"

gk_repository *gk_repository_new();
void gk_repository_init(gk_repository *repository, const char *remote_url, const char *local_path, const char *user, gk_repository_progress_callback *progress_callback, gk_repository_state_changed_callback *state_changed_callback);
void gk_repository_free(gk_repository *repository);


void gk_repository_set_last_result(gk_repository *repository, gk_result *last_result);
void gk_repository_set_last_result_v(gk_repository *repository, int code, const char *message, ...);
void gk_repository_set_last_result_vargs(gk_repository *repository, int code, const char *message, va_list args);
int gk_repository_failure(gk_repository *repository, log_Component *component, int code, const char *message, ...);
int gk_repository_success(gk_repository *repository);

int gk_repository_open_local_repository(gk_repository *repository);

// return true if ALL states are enabled
int gk_repository_state_enabled(gk_repository *repository, unsigned int states);
// return true if ALL states are disabled
int gk_repository_state_disabled(gk_repository *repository, unsigned int states);
void gk_repository_state_set(gk_repository *repository, int states_enable);
void gk_repository_state_unset(gk_repository *repository, int states_disable);
void gk_repository_state_trigger_callback(gk_repository *repository);
int gk_repository_prepend_repository_path(gk_repository *repository, char *buffer, size_t buffer_length, const char *path, const char *purpose);

typedef enum {
    GK_REPOSITORY_VERIFY_DEFAULT = (1 << 0),
    GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT = (1 << 1),
    GK_REPOSITORY_VERIFY_STATUS_LIST = (1 << 2),
    GK_REPOSITORY_VERIFY_MERGE_IN_PROGRESS = (1 << 3)
} gk_repository_verify_condition;

int gk_repository_verify(gk_repository *repository, log_Component *component, int condition, const char *purpose);


int gk_repository_context_success(gk_repository *repository, const char *purpose);
void gk_repository_context_push(gk_repository *repository, const char *purpose, log_Component *log_component);
void gk_repository_context_pop(gk_repository *repository, const char *purpose);
int gk_repository_context_failure(gk_repository *repository, const char *purpose, int code, const char *message, ...);
int gk_repository_context_lg2_failure(gk_repository *repository, const char *purpose, int code);
int gk_repository_context_lg2_failure_ex(gk_repository *repository, const char *purpose, int code, const char *message, ...);
int gk_repository_context_succeeded(gk_repository *repository);

#endif // __GITKEBAB_SESSION_H__
