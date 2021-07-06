

#ifndef __GITKEBAB_SESSION_H__
#define __GITKEBAB_SESSION_H__

#include <stdlib.h>

#include "gk_results.h"
#include "gk_credentials.h"
#include "gk_session_progress.h"
#include "gk_logging.h"

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
    size_t count_new;
    size_t count_modified;
    size_t count_deleted;
    size_t count_renamed;
    size_t count_typechange;
    size_t count_conflicted;

} gk_status_summary_t;

typedef struct {
    gk_session_progress_callback_t *progress_callback;
} gk_session_callbacks_t;

typedef struct {
    gk_repository_t *repository;
    gk_result_t *last_result;
    gk_session_callbacks_t callbacks;
    gk_session_state_t state;
    gk_status_summary_t status_summary;

    void *lg2_status_list;
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
void gk_session_set_last_result_v(gk_session_t *session, int code, const char *message, ...);
void gk_session_set_last_result_vargs(gk_session_t *session, int code, const char *message, va_list args);
int gk_session_failure(gk_session_t *session, log_Component *component, int code, const char *message, ...);
int gk_session_success(gk_session_t *session);

int gk_session_clone(gk_session_t *session, gk_session_credential_t *credential);
int gk_session_open_local_repository(gk_session_t *session);

void gk_session_free_members(gk_session_t *session);

int gk_session_credential_callback(void **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload);

#endif // __GITKEBAB_SESSION_H__
