

#ifndef __GITKEBAB_SESSION_H__
#define __GITKEBAB_SESSION_H__

#include "gk_results.h"
#include "gk_credentials.h"
#include <stdlib.h>

typedef struct {
    int network_percent;
    int index_percent;
    size_t kbytes_received;
    int deltas_resolved_percent;
} gk_fetch_progress_t;
    
typedef struct {
    size_t completed_steps;
    size_t total_steps;
    int checkout_percent;
    const char* current_path;
} gk_checkout_progress_t;

typedef struct {
    gk_fetch_progress_t fetch_progress;
    gk_checkout_progress_t checkout_progress;
    int progress_event_type;
    int percent;
    const char description[256];
} gk_session_progress_t;

typedef void gk_session_progress_callback_t(gk_session_progress_t *progress);

typedef struct {
    const char *local_path;
    const char *remote_url;
    const char *user;
} gk_repository_t;

typedef struct {
    gk_session_progress_callback_t *progress_callback;
} gk_session_callbacks_t;

typedef struct {
    gk_repository_t *repository;
    gk_result_t *last_result;
    gk_session_callbacks_t callbacks;
} gk_session_t;

typedef struct {
    gk_session_t *session;
    gk_session_credential_t *credential;
} gk_authenticated_session_t;



    
#define PROGRESS_EVENT_TYPE_FETCH 0;
#define PROGRESS_EVENT_TYPE_CHECKOUT 1;

void gk_authenticated_session_init(gk_authenticated_session_t *authed_session, gk_session_t *session, gk_session_credential_t *credential);
                                   
void gk_repository_init(gk_repository_t *repository, const char *local_path, const char *remote_url, const char *usr);

void gk_session_init(gk_session_t *session, gk_repository_t *repository, gk_session_progress_callback_t *progress_callback);
void gk_session_set_last_result(gk_session_t *session, gk_result_t *last_result);

void gk_session_clone(gk_session_t *session, gk_session_credential_t *credential);

#endif // __GITKEBAB_SESSION_H__
