
#include <string.h>

#include "gk_session.h"
#include "gk_repository.h"
#include "gk_execution_context.h"
#include "gk_logging.h"
#include "gk_init.h"
#include "gk_lg2_private.h"
#include "gk_results.h"

static gk_result *gk_session_internal_last_result(gk_session *session) {
    if (session->internal_last_result != NULL) {
        return session->internal_last_result;
    }
    
    int num_contexts = gk_execution_context_stack_size(session->context);
    if (num_contexts == 1) {
        return gk_result_success();
    }
    size_t message_length = 0;
    gk_execution_context *context = session->context->child_context;
    int result_code = GK_ERR;
    while (context != NULL) {
        message_length += strlen(context->purpose) + 16; // room for prefix
        if (context->result != NULL) {
            message_length += strlen(gk_result_message(context->result)) + 16; // room for '(error: nnnn)' suffix
        }
        if (context->result != NULL) {
            result_code = result_code == GK_ERR ? context->result->code : result_code;
        }
        context = context->child_context;
    }
    char *message = (char *)malloc(message_length);
    u_int64_t offset = 0;

    const char *prefix = "Cannot";
    context = session->context->child_context;
    while (context != NULL) {
        if (context->result != NULL) {
            offset += snprintf((char *)((u_int64_t)(message) + offset), message_length - offset - 1, "%s %s, %s (error %d)\n", prefix, context->purpose, gk_result_message(context->result), gk_result_code(context->result));
        }
        else {
            offset += snprintf((char *)((u_int64_t)(message) + offset), message_length - offset - 1, "%s %s\n", prefix, context->purpose);
        }
        offset -= 1; // don't null-terminate for now
        context = context->child_context;
        prefix = " -> failed to";
    }
    message[offset] = '\0'; // null terminate
    
    session->internal_last_result = gk_result_new(result_code, message);
    return session->internal_last_result;
}

static void gk_session_clear_internal_last_result(gk_session *session) {
    gk_result_free(session->internal_last_result);
    session->internal_last_result = NULL;
}

gk_session *gk_session_new(const char *source_url, gk_repository_source_url_type source_url_type, const char *local_path, const char *user, gk_session_progress_callback *progress_callback, gk_repository_state_changed_callback *state_changed_callback) {
    gk_session *session = (gk_session *)malloc(sizeof(gk_session));
    session->repository = gk_repository_new();
    gk_repository_init(session->repository, source_url, source_url_type, local_path, user, progress_callback, state_changed_callback);
    gk_session_credential_init(session);
    gk_session_credential_username_password_init(session, "", "");
    session->context = gk_execution_context_new("root context", &COMP_GENERAL);
    session->internal_last_result = gk_result_success();
    return session;
}

void gk_session_free(gk_session *session) {
    if (session == NULL) {
        return;
    }
    gk_repository_free(session->repository);
    session->repository = NULL;
    gk_session_credential_free_members(&session->credential);
    gk_execution_context_free(session->context);
    session->context = NULL;
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

int gk_session_verify(gk_session *session, int condition, const char *purpose) {
    gk_repository *repository = session->repository;
    if (gk_did_init() != 1) {
        return gk_session_failure_ex(session, purpose, GK_ERR, "Gitkebab not initialized");
    }

    if (condition & GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) {
        if (gk_repository_state_disabled(repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS)) {
            return gk_session_failure_ex(session, purpose, GK_ERR, "local checkout does not exist");
        }

        if (session->repository->lg2_resources->repository == NULL) {
            return gk_session_failure_ex(session, purpose, GK_ERR, "internal git2 repository is unexpectedly NULL");
        }
    }

    if (condition & GK_REPOSITORY_VERIFY_STATUS_LIST) {
        if (session->repository->lg2_resources->status_list == NULL) {
            return gk_session_failure_ex(session, purpose, GK_ERR, "must query status list first");
        }
    }

    if (condition & GK_REPOSITORY_VERIFY_MERGE_IN_PROGRESS) {
        if (session->repository->lg2_resources->merge_index == NULL) {
            return gk_session_failure_ex(session, purpose, GK_ERR, "no merge is in progress");
        }
    }

    return GK_SUCCESS;
}


int gk_session_context_push(gk_session *session, const char *purpose, log_Component *log_component, int conditions) {
    if (gk_session_context_sanity_check(session, log_component, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    gk_session_clear_internal_last_result(session);
    gk_execution_context_push(session->context, purpose, log_component);
    if ((conditions != 0) && (gk_session_verify(session, conditions, purpose) != GK_SUCCESS)) {
        return gk_session_failure(session);
    }
    return GK_SUCCESS;
}

void gk_session_context_pop(gk_session *session, const char *purpose) {
    if (gk_session_context_sanity_check(session, &COMP_REPOSITORY, "pop repository execution context") != GK_SUCCESS) {
        return;
    }
    gk_execution_context_pop(session->context, purpose);
}

int gk_session_success(gk_session *session, const char *purpose) {
    gk_execution_context_pop(session->context, purpose);
    return GK_SUCCESS;
}

int gk_session_failure(gk_session *session) {
    gk_execution_context *last_child = gk_execution_context_last_child(session->context);
    log_log(LOG_ERROR, __FILE__, __LINE__, last_child->log_component, "Failed to %s", last_child->purpose);
    return GK_FAILURE;
}

int gk_session_failure_ex(gk_session *session, const char *purpose, int code, const char *message, ...) {
    va_list args;
    va_start(args, message);
    gk_result *result = gk_result_vargs(code, message, args);
    va_end(args);
    //printf("DBG X2 generated message [%s]\n", formatted_message);
    gk_execution_context *last_child = gk_execution_context_last_child(session->context);
    gk_execution_context_set_result(last_child, result);
    log_log(LOG_ERROR, __FILE__, __LINE__, last_child->log_component, "Cannot %s: %s", purpose, gk_result_message(result));
    return GK_FAILURE;
}

int gk_session_lg2_failure(gk_session *session, const char *purpose, int code) {
    const git_error *err = git_error_last();
    return gk_session_failure_ex(session, purpose, code, "%s (error %d)", err->message, err->klass);
}

int gk_session_lg2_failure_ex(gk_session *session, const char *purpose, int code, const char *message, ...) {
    va_list args;
    va_start(args, message);
    char formatted_message[256];
    vsnprintf(formatted_message, 256, message, args);
    va_end(args);
    const git_error *err = git_error_last();
    return gk_session_failure_ex(session, purpose, code, "%s: %s (error %d)", formatted_message, err->message, err->klass);
}

gk_result *gk_session_last_result(gk_session *session) {
    if (gk_session_context_sanity_check(session, &COMP_GENERAL, "get repository last result") != GK_SUCCESS) {
        return 0;
    }
    if (session->context->child_context == NULL) {
        return session->context->result;
    }
    gk_execution_context *next_context = session->context;
    while (next_context->child_context != NULL) {
        next_context = next_context->child_context;
        if (next_context->result != NULL) {
            return next_context->result;
        }
    }
    session->context->child_context->result = gk_result_new(GK_ERR, "<unknown error>");
    return session->context->child_context->result;
}

const char *gk_session_last_result_message(gk_session *session) {
    gk_result *result = gk_session_internal_last_result(session);
    return gk_result_message(result);
}

int gk_session_last_result_code(gk_session *session) {
    gk_result *result = gk_session_internal_last_result(session);
    return gk_result_code(result);
}
