

#ifndef __GITKEBAB_SESSION_H__
#define __GITKEBAB_SESSION_H__

#include <stdlib.h>

#include "gk_results.h"
#include "gk_credentials.h"
#include "gk_session_progress.h"
#include "gk_logging.h"
#include "gk_types.h"

gk_session *gk_session_new();
void gk_session_init(gk_session *session, const char *remote_url, const char *local_path, const char *user, gk_session_progress_callback *progress_callback, gk_session_state_changed_callback *state_changed_callback);
void gk_session_free(gk_session *session);


void gk_session_set_last_result(gk_session *session, gk_result *last_result);
void gk_session_set_last_result_v(gk_session *session, int code, const char *message, ...);
void gk_session_set_last_result_vargs(gk_session *session, int code, const char *message, va_list args);
int gk_session_failure(gk_session *session, log_Component *component, int code, const char *message, ...);
int gk_session_success(gk_session *session);

int gk_session_open_local_repository(gk_session *session);

void gk_authenticated_session_init(gk_authenticated_session *authed_session, gk_session *session, gk_session_credential *credential);


// return true if ALL states are enabled
int gk_session_state_enabled(gk_session *session, int states);
// return true if ALL states are disabled
int gk_session_state_disabled(gk_session *session, int states);
void gk_session_state_set(gk_session *session, int states_enable);
void gk_session_state_unset(gk_session *session, int states_disable);
void gk_session_state_trigger_callback(gk_session *session);

#endif // __GITKEBAB_SESSION_H__
