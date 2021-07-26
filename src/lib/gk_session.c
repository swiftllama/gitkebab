
#include "gk_session.h"



gk_session *gk_session_new(const char *remote_url, const char *local_path, const char *user, gk_repository_progress_callback *progress_callback, gk_repository_state_changed_callback *state_changed_callback) {
    gk_session *session = (gk_session *)malloc(sizeof(gk_session));
    session->repository = gk_repository_new();
    gk_repository_init(session->repository, remote_url, local_path, user, progress_callback, state_changed_callback);
    gk_session_credential_username_password_init(session, "", "");
}

void gk_session_free(gk_session *session) {
    if (session == NULL) {
        return;
    }
    gk_repository_free(session->repository);
    session->repository = NULL;
    gk_session_credential_free_members(&session->credential);
    free(session);
}
int gk_session_context_sanity_check(gk_session *session, log_Component *component, const char *purpose) {
    if (session == NULL) {
        log_log(LOG_ERROR, __FILE__, __LINE__, component, "Cannot %s, session is NULL", purpose);
        return GK_FAILURE;
    }
    else if (session->context == NULL) {
        log_log(LOG_ERROR, __FILE__, __LINE__, component, "Cannot %s, session context is NULL", purpose);
        return GK_FAILURE;
    }
    else if (session->repository == NULL) {
        log_log(LOG_ERROR, __FILE__, __LINE__, component, "Cannot %s, session repository is NULL", purpose);
        return GK_FAILURE;
    }
    return GK_SUCCESS;
}

int gk_session_verify(gk_session *session, log_Component *component, int condition, const char *purpose) {
    gk_repository *repository = session->repository;
    if (gk_did_init() != 1) {
        return gk_session_failure_ex(session, purpose, GK_ERR, "Gitkebab not initialized");
    }

    if (condition & GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) {
        if (gk_repository_state_disabled(repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS)) {
            return gk_session_failure_ex(session, purpose, GK_ERR, "local checkout does not exist");
        }

        if (repository->lg2_resources->repository == NULL) {
            return gk_session_failure_ex(session, purpose, GK_ERR, "internal git2 repository is unexpectedly NULL");
        }
    }

    if (condition & GK_REPOSITORY_VERIFY_STATUS_LIST) {
        if (repository->lg2_resources->status_list == NULL) {
            return gk_session_failure_ex(session, purpose, GK_ERR, "must query status list first");
        }
    }

    if (condition & GK_REPOSITORY_VERIFY_MERGE_IN_PROGRESS) {
        if (repository->lg2_resources->merge_index == NULL) {
            return gk_session_failure_ex(session, purpose, GK_ERR, "no merge is in progress");
        }
    }

    return GK_SUCCESS;
}


int gk_session_context_push(gk_session *session, const char *purpose, log_Component *log_component, int conditions) {
    if (gk_session_context_sanity_check(repository, log_component, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    gk_execution_context_push(session->context, purpose, log_component);
    if ((conditions != 0) && (gk_session_verify(repository, &COMP_REPOSITORY, conditions, purpose) != GK_SUCCESS)) {
        return gk_session_failure(session);
    }
    return GK_SUCCESS;
}

void gk_session_context_pop(gk_session *session, const char *purpose) {
    if (gk_session_context_sanity_check(session, &COMP_REPOSITORY, "pop repository execution context") != GK_SUCCESS) {
        return;
    }
    gk_execution_context_pop(session->context, purpose);
}y

int gk_session_context_success(gk_session *session, const char *purpose) {
    gk_execution_context_pop(session->context, purpose);
    return GK_SUCCESS;
}

int gk_session_failure(gk_session *session) {
    return GK_FAILURE;
}

int gk_session_failure_ex(gk_session *session, const char *purpose, int code, const char *message, ...) {
    va_list args;
    va_start(args, message);
    gk_result *result = gk_result_v(code, message, args);
    va_end(args);
    gk_execution_context *last_child = gk_execution_context_last_child(session->context);
    if (strcmp(last_child->purpose, purpose) != 0) {
        log_error(COMP_EXCTX, "Failure recorded for purpose [%s], but last child has different purpose [%s]", purpose,  last_child->purpose);
    }
    gk_execution_context_set_result(last_child, result);
    return GK_FAILURE;
}

int gk_session_lg2_failure(gk_session *session, const char *purpose, int code) {
    const git_error *err = git_error_last();
    return gk_session_failure_ex(session, purpose, code, "%s (error %d)", err->message, err->klass);
}

int gk_session_context_lg2_failure_ex(gk_session *session, const char *purpose, int code, const char *message, ...) {
    va_list args;
    va_start(args, message);
    char formatted_message[256];
    vsnprintf(formatted_message, 256, message, args);
    va_end(args);
    const git_error *err = git_error_last();
    return gk_session_failure_ex(session, purpose, code, "%s: %s (error %d)", formatted_message, err->message, err->klass);
}

int gk_session_context_succeeded(gk_session *session) {
    if (gk_session_verify(repository, &COMP_REPOSITORY, GK_SESSION_VERIFY_DEFAULT, "check repository result success") != GK_SUCCESS) {
        return 0;
    }
    if (repository->root_context->child_context == NULL) {
        return 1;
    }
    return 0;
}
