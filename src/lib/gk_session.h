
#ifndef __GK_SESSION_H__
#define __GK_SESSION_H__

#include "gk_types.h"

gk_session *gk_session_new(const char *remote_url, const char *local_path, const char *user, gk_repository_progress_callback *progress_callback, gk_repository_state_changed_callback *state_changed_callback);
void gk_session_free(gk_session *session);
int gk_session_context_sanity_check(gk_session *session, log_Component *component, const char *purpose);
int gk_session_verify(gk_session *session, log_Component *component, int condition, const char *purpose);
int gk_session_context_push(gk_session *session, const char *purpose, log_Component *log_component, int conditions);
void gk_session_context_pop(gk_session *session, const char *purpose);
int gk_session_context_success(gk_session *session, const char *purpose);
int gk_session_failure(gk_session *session);
int gk_session_failure_ex(gk_session *session, const char *purpose, int code, const char *message, ...);
int gk_session_lg2_failure(gk_session *session, const char *purpose, int code);
int gk_session_context_lg2_failure_ex(gk_session *session, const char *purpose, int code, const char *message, ...);
int gk_session_context_succeeded(gk_session *session);

#endif __GK_SESSION_H__
