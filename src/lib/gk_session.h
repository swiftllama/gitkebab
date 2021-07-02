

#ifndef __GITKEBAB_SESSION_H__
#define __GITKEBAB_SESSION_H__

#include <stdlib.h>

#include "gk_results.h"
#include "gk_credentials.h"
#include "gk_session_progress.h"

typedef struct {
    short int local_checkout_exists;
    short int has_conflicts;
    short int merge_in_progress;
    short int clone_in_progress;
    short int push_in_progress;
    short int pull_in_progress;
} gk_session_state_t;
    
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
    gk_session_state_t state;
    void *lg2_repository;
} gk_session_t;

typedef struct {
    gk_session_t *session;
    gk_session_credential_t *credential;
} gk_authenticated_session_t;


void gk_authenticated_session_init(gk_authenticated_session_t *authed_session, gk_session_t *session, gk_session_credential_t *credential);
                                   
void gk_repository_init(gk_repository_t *repository, const char *local_path, const char *remote_url, const char *usr);

void gk_session_init(gk_session_t *session, gk_repository_t *repository, gk_session_progress_callback_t *progress_callback);
void gk_session_set_last_result(gk_session_t *session, gk_result_t *last_result);

void gk_session_clone(gk_session_t *session, gk_session_credential_t *credential);
void gk_session_open_local_repository(gk_session_t *session);

void gk_session_free_members(gk_session_t *session);

#endif // __GITKEBAB_SESSION_H__
