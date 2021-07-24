
#include "stdio.h"
#include <string.h>

#include "git2.h"
#include "gk_session.h"
#include "gk_results.h"
#include "gk_logging.h"
#include "gk_status.h"
#include "gk_conflicts.h"
#include "gk_lg2_private.h"
#include "gk_filesystem.h"
#include "gk_execution_context.h"

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
    repository->main_branch_name = "master";
    repository->remote_ref_name = "refs/remotes/origin/master";
    repository->remote_name = "origin";
    repository->push_refspec = "refs/heads/master:refs/heads/master";
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
    gk_session *session = (gk_session *)malloc(sizeof(gk_session));
    session->lg2_resources = (gk_lg2_resources *)malloc(sizeof(gk_lg2_resources));
    return session;
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
    session->root_context = gk_execution_context_new("root context", &COMP_SESSION);
    session->last_result = gk_result_success();
    session->callbacks.progress_callback = progress_callback;
    session->callbacks.state_changed_callback = state_changed_callback;

    session->state = 0;
    gk_status_summary_reset(&session->status_summary);
    
    gk_lg2_resources_init(session);

    session->conflict_summary.num_conflicts = 0;
    session->conflict_summary.conflicts = NULL;
}

int gk_session_open_local_repository(gk_session *session) {
    if (session == NULL) {
        log_error(COMP_SESSION, "gk_session_open_local_repository(session) called on NULL session");
        return GK_FAILURE;
    }

    if (gk_lg2_repository_open(session, "open local repository") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_session_state_set(session, GK_SESSION_STATE_LOCAL_CHECKOUT_EXISTS);

    if (git_repository_is_bare(session->lg2_resources->repository) == 1) {
        gk_session_state_unset(session, GK_SESSION_STATE_HAS_CONFLICTS);
        gk_session_state_unset(session, GK_SESSION_STATE_MERGE_PENDING_ON_DISK);
    }
    else {
        int rc = gk_session_status_summary_query(session);
        gk_session_status_summary_close(session);
        if (rc != GK_SUCCESS) {
            return GK_FAILURE;
        }
    }

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
    gk_execution_context_free(session->root_context);
    session->root_context = NULL;
    gk_repository_free_members(&session->repository);
    gk_conflicts_free(session);
    gk_lg2_free_all_but_repository(session);
    free(session->lg2_resources);
    free(session);
}

int gk_session_state_enabled(gk_session *session, unsigned int states) {
    if (session == NULL) {
        log_warn(COMP_SESSION, "gk_session_state_enabled called on NULL session");
        return 0;
    }
    return (session->state & states) == states;
}

int gk_session_state_disabled(gk_session *session, unsigned int states) {
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

int gk_session_verify(gk_session *session, log_Component *component, int condition, const char *purpose) {
    if (session == NULL) {
        log_log(LOG_ERROR, __FILE__, __LINE__, component, "Cannot %s, session is NULL", purpose);
        return GK_FAILURE;
    }
    else if (session->root_context == NULL) {
        log_log(LOG_ERROR, __FILE__, __LINE__, component, "Cannot %s, session root context is NULL", purpose);
        return GK_FAILURE;
    }

    if (condition & GK_SESSION_VERIFY_LOCAL_CHECKOUT) {
        if (gk_session_state_disabled(session, GK_SESSION_STATE_LOCAL_CHECKOUT_EXISTS)) {
            return gk_session_failure(session, component, -3, "Cannot %s, local checkout does not exist", purpose);
        }

        if (session->lg2_resources->repository == NULL) {
            return gk_session_failure(session, component, -4, "Cannot %s, internal git2 repository is unexpectedly NULL", purpose);
        }
    }

    if (condition & GK_SESSION_VERIFY_STATUS_LIST) {
        if (session->lg2_resources->status_list == NULL) {
            return gk_session_failure(session, component, -5, "Cannot %s, must query status list first", purpose);
        }
    }

    if (condition & GK_SESSION_VERIFY_MERGE_IN_PROGRESS) {
        if (session->lg2_resources->merge_index == NULL) {
            return gk_session_failure(session, component, -5, "Cannot %s, no merge is in progress", purpose);
        }
    }

    return GK_SUCCESS;
}

int gk_session_prepend_repository_path(gk_session *session, char *buffer, size_t buffer_length, const char *path, const char *purpose) {
    if (gk_session_verify(session, &COMP_SESSION, GK_SESSION_VERIFY_LOCAL_CHECKOUT, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    if (buffer == NULL) {
        return gk_session_failure(session, &COMP_SESSION, -4, "Error prepending repository path to [%s], destination buffer is NULL", path);
    }

    if (gk_concatenate_paths(buffer, buffer_length, session->repository.local_path, path) != 0) {
        return gk_session_failure(session, &COMP_SESSION, -4, "Error prepending repository path [%s] to [%s] NULL", session->repository.local_path, path);
    }
    
    return GK_SUCCESS;
}

int gk_session_result_success(gk_session *session) {
    if (gk_session_verify(session, &COMP_SESSION, GK_SESSION_VERIFY_DEFAULT, "check session result success") != GK_SUCCESS) {
        return 0;
    }
    if (session->root_context->child_context == NULL) {
        return 1;
    }
    return 0;
}

void gk_session_context_push(gk_session *session, const char *purpose, log_Component *log_component) {
    if (gk_session_verify(session, &COMP_SESSION, GK_SESSION_VERIFY_DEFAULT, "push session execution context") != GK_SUCCESS) {
        return;
    }
    gk_execution_context_push(session->root_context, purpose, log_component);
}

void gk_session_context_pop(gk_session *session, const char *purpose) {
    if (gk_session_verify(session, &COMP_SESSION, GK_SESSION_VERIFY_DEFAULT, "pop session execution context") != GK_SUCCESS) {
        return;
    }
    gk_execution_context_pop(session->root_context, purpose);
}
