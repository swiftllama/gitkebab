
#include "stdio.h"
#include <string.h>

#include "git2.h"
#include "gk_session.h"
#include "gk_results.h"
#include "gk_logging.h"
#include "gk_status.h"

static void gk_repository_init(gk_repository *repository, const char *remote_url, const char *local_path, const char *usr) {
    if (repository == NULL) {
        return;
    }
    if (remote_url == NULL) {
        log_warn(COMP_SESSION, "Repository initialized with NULL remote_url, will use empty string instead");
    }
    if (local_path == NULL) {
        log_warn(COMP_SESSION, "Repository initialized with NULL local_url, will use empty string instead");
    }
    if (usr == NULL) {
        log_warn(COMP_SESSION, "Repository initialized with NULL user, will use empty string instead");
    }
    repository->local_path = local_path != NULL ? strdup(local_path) : strdup("");
    repository->remote_url = remote_url != NULL ? strdup(remote_url) : strdup("");
    repository->user = usr != NULL ? strdup(usr) : strdup("");
}

static void gk_repository_free_members(gk_repository *repository) {
    free((char *)repository->local_path);
    repository->local_path = NULL;
    free((char *)repository->remote_url);
    repository->remote_url = NULL;
    free((char *)repository->user);
    repository->user = NULL;
}

gk_session *gk_session_new() {
    return (gk_session *)malloc(sizeof(gk_session));
}


void gk_authenticated_session_init(gk_authenticated_session *authed_session, gk_session *session, gk_session_credential *credential) {
    if (authed_session == NULL) {
        return;
    }
    authed_session->session = session;
    authed_session->credential = credential;
}

void gk_session_init(gk_session *session, const char *remote_url, const char *local_path, const char *user, gk_session_progress_callback *progress_callback, gk_session_state_changed_callback *state_changed_callback) {
    if (session == NULL) {
        return;
    }
    gk_repository_init(&session->repository, remote_url, local_path, user);
    session->last_result = gk_result_success();
    session->callbacks.progress_callback = progress_callback;
    session->callbacks.state_changed_callback = state_changed_callback;

    session->state = 0;
    gk_status_summary_reset(&session->status_summary);
    
    session->lg2_repository = NULL;
    session->lg2_status_list = NULL;
}

int gk_session_open_local_repository(gk_session *session) {
    gk_result *result = NULL;
    
    if (session == NULL) {
        log_error(COMP_SESSION, "gk_session_open_local_repository(session) called on NULL session");
        return GK_FAILURE;
    }

    int rc = git_repository_open((git_repository **)&(session->lg2_repository), session->repository.local_path);
    if (rc != 0) {
        git_repository_free(session->lg2_repository);
        session->lg2_repository = NULL;
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_SESSION, -2, "Error opening repository at local path (%d): %s", err->klass, err->message);
    }

    gk_session_state_set(session, GK_SESSION_STATE_LOCAL_CHECKOUT_EXISTS);
    return gk_session_success(session);
}

void gk_session_set_last_result(gk_session *session, gk_result *last_result) {
    gk_result_free(session->last_result);
    session->last_result = last_result;
    if (last_result == NULL) {
        log_error(COMP_SESSION, "Set a NULL last error on a session, this could be the result of a memory allocation failure");
    }
}

void gk_session_set_last_result_v(gk_session *session, int code, const char *message, ...) {
    va_list(args);
    va_start(args, message);
    gk_result *result = gk_result_v(code, message, args);
    va_end(args);
    gk_session_set_last_result(session, result);
}

void gk_session_set_last_result_vargs(gk_session *session, int code, const char *message, va_list args) {
    gk_result *result = gk_result_vargs(code, message, args);
    gk_session_set_last_result(session, result);
}

int gk_session_failure(gk_session *session, log_Component *component, int code, const char *message, ...) {
    va_list args;
    va_start(args, message);
    gk_session_set_last_result_vargs(session, code, message, args);
    va_end(args);
    log_log(LOG_ERROR, __FILE__, __LINE__, component, gk_result_message(session->last_result));
    return GK_FAILURE;
}

int gk_session_success(gk_session *session) {
    gk_session_set_last_result(session, gk_result_success());
    return GK_SUCCESS;
}

void gk_session_free(gk_session *session) {
    if (session == NULL) {
        return;
    }

    gk_result_free(session->last_result);
    session->last_result = NULL;
    git_status_list_free(session->lg2_status_list);
    git_repository_free(session->lg2_repository);
    gk_repository_free_members(&session->repository);
    free(session);
}

int gk_session_state_enabled(gk_session *session, int states) {
    if (session == NULL) {
        log_warn(COMP_SESSION, "gk_session_state_enabled called on NULL session");
        return 0;
    }
    return (session->state & states) == states;
}

int gk_session_state_disabled(gk_session *session, int states) {
    if (session == NULL) {
        log_warn(COMP_SESSION, "gk_session_state_disabled called on NULL session");
        return 0;
    }
    return (session->state & states) == 0;
}

void gk_session_state_set(gk_session *session, int states_enable) {
    if (session == NULL) {
        log_warn(COMP_SESSION, "gk_session_state_set called on NULL session");
        return;
    }
    session->state |= states_enable;
}

void gk_session_state_unset(gk_session *session, int states_disable) {
    if (session == NULL) {
        log_warn(COMP_SESSION, "gk_session_state_unset called on NULL session");
        return;
    }
    session->state &= ~states_disable;
}

void gk_session_state_trigger_callback(gk_session *session) {
    if (session == NULL) {
        log_warn(COMP_SESSION, "gk_session_state_trigger_callback called on NULL session");
        return;
    }
    session->callbacks.state_changed_callback(session);
}
