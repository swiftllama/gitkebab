
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "gk_session.h"
#include "gk_repository.h"
#include "gk_status.h"
#include "gk_execution_context.h"
#include "gk_logging.h"
#include "gk_init.h"
#include "gk_lg2_private.h"
#include "gk_results.h"
#include "gk_filesystem.h"
#include <time.h>

static int _session_id_counter = 0;

static void gk_session_generate_uid(gk_session *session) {
    // NOTE: replace with real UUIDs?
    time_t rawtime;
    time(&rawtime);
    // NOTE: on iOS, time_t is not equivalent to intmax_t - it casts
    // to long rather than long long. So cast to intmax_t for safety
    snprintf(session->id, 16, "%d%jd", _session_id_counter, (intmax_t)rawtime);
    _session_id_counter += 1;
    session->id_ptr = session->id;
}

static int gk_session_init_state_lock(gk_session *session) {
    const char *purpose = "create state lock";
    if (gk_session_context_push(session, purpose, &COMP_SESSION, GK_REPOSITORY_VERIFY_NONE) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    session->state_lock = malloc(sizeof(pthread_mutex_t));
    if (session->state_lock == NULL) {
        return gk_session_failure_ex(session, purpose, GK_ERR, "failed to allocate memory for state lock");
    }
    int rc = pthread_mutex_init((pthread_mutex_t *)session->state_lock, NULL);
    if (rc != 0){
        free(session->state_lock);
        session->state_lock = NULL;
        return gk_session_failure_ex(session, purpose, rc, "failed to initialize state lock (error %d)", rc);
    }
    return gk_session_success(session, purpose);
}

static void gk_session_destroy_state_lock(gk_session *session) {
    if (session->state_lock == NULL) return;
    pthread_mutex_destroy((pthread_mutex_t *)session->state_lock);
    free(session->state_lock);
    session->state_lock = NULL;
}


gk_session *gk_session_new(const char *source_url, gk_repository_source_url_type source_url_type, const char *local_path, const char *user, gk_repository_state_changed_callback *state_changed_callback, gk_repository_did_query_merge_conflict_summary *merge_conflict_query_callback) {
    gk_session *session = (gk_session *)malloc(sizeof(gk_session));
    memset(session, 0, sizeof(gk_session));
    gk_session_generate_uid(session);
    session->repository = gk_repository_new();
    gk_repository_init(session->repository, source_url, source_url_type, local_path, user);
    gk_session_credential_init(session);
    gk_session_credential_username_password_init(session, "", "");
    session->progress = NULL;
    session->context = gk_execution_context_new("root context", &COMP_GENERAL);
    gk_session_init_state_lock(session); // must come after session->context is created
    session->internal_last_result = gk_result_success();
    session->callbacks.state_changed_callback = state_changed_callback;
    session->callbacks.merge_conflict_query_callback = merge_conflict_query_callback;
    return session;
}

void gk_session_free(gk_session *session) {
    if (session == NULL) {
        return;
    }
    gk_repository_free(session->repository);
    session->repository = NULL;
    gk_session_credential_free_members(&session->credential);
    gk_session_progress_free(session->progress);
    gk_session_destroy_state_lock(session);
    gk_execution_context_free(session->context);
    session->context = NULL;
    free(session);
}

int gk_session_initialize(gk_session *session) {
    const char *purpose = "initialize session";
    if (gk_session_context_push(session, purpose, &COMP_SESSION, GK_REPOSITORY_VERIFY_NONE) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    log_info(COMP_SESSION, "Initializing session");
    if (gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_INITIALIZED)) {
        log_warn(COMP_SESSION, "Session already initialized");
        return gk_session_success(session, purpose);
    }

    if (gk_directory_exists(session->repository->spec.local_path) == 0) {
        log_info(COMP_SESSION, "Local directory [%s] does not yet exist", session->repository->spec.local_path);
        gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_INITIALIZED);
    }
    else {
        log_info(COMP_SESSION, "Local directory [%s] already exists", session->repository->spec.local_path); 
        if (gk_open_local_repository(session) != 0) {
            const git_error *last_error = git_error_last();
            if (last_error->klass == GIT_ERROR_REPOSITORY) {
                int dot_git_exists = gk_subdirectory_exists(session->repository->spec.local_path, ".git");
                int objects_exists = gk_subdirectory_exists(session->repository->spec.local_path, "objects");
                int refs_exists = gk_subdirectory_exists(session->repository->spec.local_path, "refs");
                if ((dot_git_exists == 0) || ((objects_exists == 0) && (refs_exists == 0))) {
                    return gk_session_failure_ex(session, purpose, GK_ERR_REPOSITORY_LOCAL_PATH_CONFLICT, "local path [%s] exists but it does not seem to be a git repository", session->repository->spec.local_path);
                }
            }
            return gk_session_failure_ex(session, purpose, GK_ERR, "Failed to open repo at local path [%s]", session->repository->spec.local_path);
        }

        gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_INITIALIZED);
        if (git_repository_is_bare(session->repository->lg2_resources->repository) == 0) {
            int rc = gk_status_summary_query(session);
            gk_status_summary_close(session);
            if (rc != GK_SUCCESS) {
                return gk_session_failure_ex(session, purpose, GK_ERR, "failed to query status");
            }
        }
    }

    if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }

    return gk_session_success(session, purpose);
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

    if ((condition & GK_REPOSITORY_VERIFY_INITIALIZED) || (condition & GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT)) {
        if (gk_repository_state_disabled(repository, GK_REPOSITORY_STATE_INITIALIZED)) {
            return gk_session_failure_ex(session, purpose, GK_ERR_REPOSITORY_NOT_INITIALIZED, "repository not initialized");
        }
    }

    if (condition & GK_REPOSITORY_VERIFY_STATE_LOCK) {
        if (session->state_lock == NULL) {
            return gk_session_failure_ex(session, purpose, GK_ERR_REPOSITORY_NOT_INITIALIZED, "state lock is NULL");
        }
    }
    
    if (condition & GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) {
        if (gk_repository_state_disabled(repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS)) {
            return gk_session_failure_ex(session, purpose, GK_ERR_REPOSITORY_NO_LOCAL_CHECKOUT, "local checkout does not exist");
        }

        if (session->repository->lg2_resources->repository == NULL) {
            return gk_session_failure_ex(session, purpose, GK_ERR_REPOSITORY_NO_LOCAL_CHECKOUT, "internal git2 repository is unexpectedly NULL");
        }
    }

    if (condition & GK_REPOSITORY_VERIFY_REPOSITORY_LOADED) {
        if (session->repository->lg2_resources->repository == NULL) {
            return gk_session_failure_ex(session, purpose, GK_ERR_REPOSITORY_NOT_LOADED, "repository is not loaded");
        }
    }

    if (condition & GK_REPOSITORY_VERIFY_INDEX_LOADED) {
        if (session->repository->lg2_resources->index == NULL) {
            return gk_session_failure_ex(session, purpose, GK_ERR, "must load index first");
        }
    }

    if (condition & GK_REPOSITORY_VERIFY_MERGE_INDEX_LOADED) {
        if (session->repository->lg2_resources->merge_index == NULL) {
            return gk_session_failure_ex(session, purpose, GK_ERR, "merge index is not loaded ");
        }
    }
    
    if (condition & GK_REPOSITORY_VERIFY_STATUS_LIST) {
        if (session->repository->lg2_resources->status_list == NULL) {
            return gk_session_failure_ex(session, purpose, GK_ERR, "must query status list first");
        }
    }

    if (condition & GK_REPOSITORY_VERIFY_MERGE_IN_PROGRESS) {
        if (gk_repository_state_disabled(session->repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS)) {
            return gk_session_failure_ex(session, purpose, GK_ERR, "no merge is in progress");
        }
    }

    return GK_SUCCESS;
}

int gk_session_set_repository_state_with_callback(gk_session *session, int states) {
    gk_repository_state_set(session->repository, states);
    return gk_session_trigger_repository_state_callback(session);
}

int gk_session_unset_repository_state_with_callback(gk_session *session, int states) {
    gk_repository_state_unset(session->repository, states);
    return gk_session_trigger_repository_state_callback(session);
}
                                                  
int gk_session_trigger_repository_state_callback(gk_session *session) {
    if (gk_session_context_sanity_check(session, &COMP_SESSION, "trigger repository state callback") != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (session->callbacks.state_changed_callback != NULL) {
        log_info(COMP_SESSION, "invoking state change callback with new repository state [%d - %s%s%s%s%s%s%s%s%s%s%s%s%s]", session->repository->state,(session->repository->state & GK_REPOSITORY_STATE_INITIALIZED) ? " initialized " : "", (session->repository->state & GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS) ? " local_checkout_exists " : "", (session->repository->state & GK_REPOSITORY_STATE_HAS_CONFLICTS) ? " has_conflicts " : "", (session->repository->state & GK_REPOSITORY_STATE_HAS_CHANGES_TO_COMMIT) ? " has_changes_to_commit " : "", (session->repository->state & GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE) ? " has_changes_to_merge " : "", (session->repository->state & GK_REPOSITORY_STATE_CLONE_IN_PROGRESS) ? " clone_in_progress " : "", (session->repository->state & GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING) ? " merge_finalization_pending " : "", (session->repository->state & GK_REPOSITORY_STATE_MERGE_PENDING_ON_DISK) ? " merge_pending_on_disk " : "", (session->repository->state & GK_REPOSITORY_STATE_PUSH_IN_PROGRESS) ? " push_in_progress " : "", (session->repository->state & GK_REPOSITORY_STATE_FETCH_IN_PROGRESS) ? " fetch_in_progress " : "", (session->repository->state & GK_REPOSITORY_STATE_MERGE_IN_PROGRESS) ? " merge_in_progress " : "", (session->repository->state & GK_REPOSITORY_STATE_SYNC_IN_PROGRESS) ? " sync_in_progress " : "", (session->repository->state & GK_REPOSITORY_STATE_BACKGROUND_SYNC_IN_PROGRESS) ? " background_sync_in_progress " : "");
        session->callbacks.state_changed_callback(session->id_ptr, session->repository, session->progress);
    }
    else {
        //log_info(COMP_SESSION, "Session's repository state change callback is NULL, no state change callback will be invoked");
    }
    return GK_SUCCESS;
}

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
    int result_code = GK_SUCCESS; // Assume success 
    while (context != NULL) {
        message_length += strlen(context->purpose) + 16; // room for prefix
        if (context->result != NULL) {
            message_length += strlen(gk_result_message(context->result)) + 16; // room for '(error: nnnn)' suffix
        }
        if (context->result != NULL) {
            // if a sub context has a result, let it override the current result code if its either a GK_SUCCESS or a GK_ERR (this last one is generic, if we can find a specific error code that's better)
            result_code = result_code == GK_SUCCESS ? context->result->code : (result_code == GK_ERR ? context->result->code : result_code);
        }
        context = context->child_context;
    }

    
    char *message = NULL;
    if (result_code != GK_SUCCESS) {
      message = (char *)malloc(message_length);
      size_t offset = 0;
  
      const char *prefix = "Cannot";
      context = session->context->child_context;
      while (context != NULL) {
          if (context->result != NULL) {
              offset += snprintf((char *)((size_t)(message) + offset), message_length - offset - 1, "%s %s\n -> %s (error %d)\n", prefix, context->purpose, gk_result_message(context->result), gk_result_code(context->result));
          }
          else {
              offset += snprintf((char *)((size_t)(message) + offset), message_length - offset - 1, "%s %s\n", prefix, context->purpose);
          }
          offset -= 1; // don't null-terminate for now
          context = context->child_context;
          prefix = " -> failed to";
      }
      message[offset] = '\0'; // null terminate
    }

    session->internal_last_result = gk_result_new(result_code, message);
    free(message);
    return session->internal_last_result;
}

static void gk_session_clear_internal_last_result(gk_session *session) {
    gk_result_free(session->internal_last_result);
    session->internal_last_result = NULL;
}

int gk_session_context_push(gk_session *session, const char *purpose, log_Component *log_component, int conditions) {
    if (gk_session_context_sanity_check(session, log_component, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    gk_session_clear_internal_last_result(session);
    log_info(COMP_EXCTX, "pushing purpose [%s]", purpose);
    gk_execution_context_push(session->context, purpose, log_component);
    if ((conditions != 0) && (gk_session_verify(session, conditions, purpose) != GK_SUCCESS)) {
        return GK_FAILURE;
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
    log_info(COMP_EXCTX, "popping purpose [%s]", purpose);
    gk_session_clear_internal_last_result(session);
    gk_execution_context_pop(session->context, purpose);
    return GK_SUCCESS;
}

int gk_session_failure(gk_session *session, const char *purpose) {
    return gk_session_failure_ex(session, purpose, GK_ERR, "");
}

int gk_session_failure_ex(gk_session *session, const char *purpose, int code, const char *message, ...) {
    va_list args;
    va_start(args, message);
    gk_result *result = gk_result_vargs(code, message, args);
    va_end(args);
    
    gk_execution_context *last_unresolved = gk_execution_context_last_unresolved(session->context, NULL);
    log_Component *log_component = last_unresolved != NULL ? last_unresolved->log_component : &COMP_GENERAL;
    log_log(LOG_ERROR, __FILE__, __LINE__, log_component, "Cannot %s", purpose);
    if (message[0] != '\0') {
        log_log(LOG_ERROR, __FILE__, __LINE__, log_component, "  -> %s", gk_result_message(result));
    }
    
    if (last_unresolved == NULL) {
        log_error(COMP_EXCTX, "Cannot set session failure, execution chain has no unresolved contexts. The most likely cause is a method that doesn't properly report its success/failure via gk_session_success() or gk_session_failure*()");
        gk_execution_context_print_execution_chain(session->context);
    }
    else if (strcmp(last_unresolved->purpose, purpose) != 0) {
        log_error(COMP_EXCTX, "Cannot set session failure for purpose [%s], the last unresolved execution context has a different purpose [%s]. The most likely cause is a method that doesn't properly report its success/failure via gk_session_success() or gk_session_failure*()", purpose, last_unresolved->purpose);
        gk_execution_context_print_execution_chain(session->context);
    }
    else {
        gk_execution_context_set_result(last_unresolved, result);
        // NOTE: this is premature optimization, better to let
        // the last internal result be calculated on an as-need basis
        //session->internal_last_result = gk_result_new(result->code, result->message);
    }
    
    return GK_FAILURE;
}

int gk_session_lg2_failure(gk_session *session, const char *purpose, int code) {
    (void) code;
    const git_error *err = git_error_last();
    if (err == NULL) {
        log_error(COMP_EXCTX, "libgit had an error, but git_error_last() returned null. The code reported to lg2_failure is %d", code);
        return gk_session_failure_ex(session, purpose, code, "unknown error #%d", code);
    }
    return gk_session_failure_ex(session, purpose, err->klass, "%s", err->message);
}

int gk_session_lg2_failure_ex(gk_session *session, const char *purpose, int code, const char *message, ...) {
    (void) code;
    va_list args;
    va_start(args, message);
    char formatted_message[256];
    vsnprintf(formatted_message, 256, message, args);
    va_end(args);
    const git_error *err = git_error_last();
    if (err == NULL) {
        log_error(COMP_EXCTX, "libgit had an error, but git_error_last() returned null. The code reported to lg2_failure_ex is %d", code);
        return gk_session_failure_ex(session, purpose, code, "%s", formatted_message);
    }    
    return gk_session_failure_ex(session, purpose, code, "%s: %s", formatted_message, err->message);
}

const char *gk_session_last_result_message(gk_session *session) {
    //gk_execution_context_print_execution_chain(session->context);
    //printf("=> last internal result: %p\n", (void *)session->internal_last_result);
    gk_result *result = gk_session_internal_last_result(session);
    return gk_result_message(result);
}

int gk_session_last_result_code(gk_session *session) {
    gk_result *result = gk_session_internal_last_result(session);
    return gk_result_code(result);
}

void gk_session_print_execution_context_chain(gk_session *session) {
    gk_execution_context_print_execution_chain(session->context);
}

int gk_session_state_lock(gk_session *session) {
    const char *purpose = "lock session state lock";
    if (gk_session_context_sanity_check(session, &COMP_SESSION, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (gk_session_verify(session, GK_REPOSITORY_VERIFY_INITIALIZED | GK_REPOSITORY_VERIFY_STATE_LOCK, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    int rc = pthread_mutex_lock((pthread_mutex_t *)session->state_lock);
    if (rc != 0) {
        log_warn(COMP_SESSION, "failed to lock state lock (error %d)", rc);
    }
    return rc == 0 ? GK_SUCCESS : rc;
}


/*
// NOTE: returns -1 on error, 0 on success (mutex is locked) and 1 if the mutex is currently locked
int gk_session_state_trylock(gk_session *session) {
    if (gk_session_context_sanity_check(session, &COMP_SESSION, purpose) != GK_SUCCESS) {
        return -1;
    }
    if (gk_session_verify(session, GK_REPOSITORY_VERIFY_INITIALIZED | GK_REPOSITORY_VERIFY_STATE_LOCK, purpose) != GK_SUCCESS) {
        return -1;
    }

    int rc = pthread_mutex_trylock((pthread_mutex_t *)session->state_lock);
    if ((rc == 0) || (rc == EAGAIN)) {
        return rc == 0 ? 0 : 1;
    }

    return -1;
}*/

int gk_session_state_unlock(gk_session *session) {
    const char *purpose = "unlock session state lock";
    if (gk_session_context_sanity_check(session, &COMP_SESSION, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (gk_session_verify(session, GK_REPOSITORY_VERIFY_INITIALIZED | GK_REPOSITORY_VERIFY_STATE_LOCK, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    int rc = pthread_mutex_unlock((pthread_mutex_t *)session->state_lock);
    if (rc == EPERM) {
        log_warn(COMP_SESSION, "failed to unlock state lock, unlock must be called from the same thread that locked the state");
        return GK_FAILURE;
    }
    if (rc != 0) {
        log_warn(COMP_SESSION, "failed to unlock state lock (error %d)", rc);
        return GK_FAILURE;
    }
    return GK_SUCCESS;
}
