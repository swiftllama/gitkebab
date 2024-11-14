#include <pthread.h>
#include <errno.h>

#include "gk_execution_context.h"
#include "gk_logging.h"
#include "gk_remotes.h"
#include "gk_merge.h"
#include "gk_session.h"
#include "gk_index.h"
#include "gk_commit.h"
#include "gk_status.h"
#include "gk_sync.h"

#define DEBUG_FORCE_LONG_SYNC 0
#if DEBUG_FORCE_LONG_SYNC
#include <unistd.h>
#endif

static void *gk_background_sync_worker(void* session_ptr) {
    gk_session *session = (gk_session *)session_ptr;
    gk_sync(session);
    // NOTE: don't trigger a callback for background sync (background syncs rely on polling not callbacks)
    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_BACKGROUND_SYNC_IN_PROGRESS);
    return NULL;
}

int gk_background_sync(gk_session* session) {
    const char *purpose = "session background sync";
    if (gk_session_context_sanity_check(session, &COMP_SYNC, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (gk_session_verify(session, GK_REPOSITORY_VERIFY_INITIALIZED | GK_REPOSITORY_VERIFY_STATE_LOCK, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    // NOTE: don't trigger a callback for background sync (background syncs rely on polling not callbacks)
    gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_BACKGROUND_SYNC_IN_PROGRESS);
    
    pthread_t thread_id;
    int rc = pthread_create(&thread_id, NULL, gk_background_sync_worker, (void *)session);
    pthread_detach(thread_id);
    
    if (rc != 0) {
        if (gk_session_context_push(session, purpose, &COMP_SYNC, GK_REPOSITORY_VERIFY_INITIALIZED | GK_REPOSITORY_VERIFY_STATE_LOCK) != GK_SUCCESS) {
            return GK_FAILURE;
        }
        if (rc == EAGAIN) {
            return gk_session_failure_ex(session, purpose, rc, "Encountered EAGAIN while starting sync background thread");
        }
        return gk_session_failure_ex(session, purpose, rc, "Error starting background sync thread");
    }
    return GK_SUCCESS;
}

int gk_sync(gk_session *session) {
    const char *purpose = "sync repository";
    gk_execution_context_print_execution_chain(session->context);
    
    if (gk_session_context_push(session, purpose, &COMP_SYNC, GK_REPOSITORY_VERIFY_INITIALIZED) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_execution_context_print_execution_chain(session->context);
    
    if (gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_SYNC_IN_PROGRESS)) {
        log_warn(COMP_SESSION, "Session sync already in progress");
        return gk_session_success(session, purpose);
    }

    if (gk_session_set_repository_state_with_callback(session, GK_REPOSITORY_STATE_SYNC_IN_PROGRESS) != GK_SUCCESS) {
        gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_SYNC_IN_PROGRESS);
        return gk_session_failure(session, purpose);
    }

    if (gk_repository_state_disabled(session->repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS)) {
        if (gk_clone(session) != GK_SUCCESS) {
            gk_session_unset_repository_state_with_callback(session, GK_REPOSITORY_STATE_SYNC_IN_PROGRESS);
            return gk_session_failure(session, purpose);
        }

        gk_session_unset_repository_state_with_callback(session, GK_REPOSITORY_STATE_SYNC_IN_PROGRESS);
        return gk_session_success(session, purpose);
    }

    if (gk_status_summary_query(session) != GK_SUCCESS) {
        gk_session_unset_repository_state_with_callback(session, GK_REPOSITORY_STATE_SYNC_IN_PROGRESS);
        return gk_session_failure(session, purpose);
    }

    if (gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_COMMIT)) {
        if (gk_index_add_all(session, ".") != GK_SUCCESS) {
            gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_SYNC_IN_PROGRESS);
            return gk_session_failure(session, purpose);
        }
        
        time_t ltime;
        struct tm result;
        char stime[64];

        ltime = time(NULL);
        localtime_r(&ltime, &result);
        asctime_r(&result, stime);
        if (gk_commit(session, stime, NULL) != GK_SUCCESS) {
            gk_session_unset_repository_state_with_callback(session, GK_REPOSITORY_STATE_SYNC_IN_PROGRESS);
            return gk_session_failure(session, purpose);
        }

        if (gk_status_summary_query(session) != GK_SUCCESS) {
            gk_session_unset_repository_state_with_callback(session, GK_REPOSITORY_STATE_SYNC_IN_PROGRESS);
            return gk_session_failure(session, purpose);
        }
    }

    if (gk_fetch(session, session->repository->spec.remote_name) != GK_SUCCESS) {
        gk_session_unset_repository_state_with_callback(session, GK_REPOSITORY_STATE_SYNC_IN_PROGRESS);
        return gk_session_failure(session, purpose);
    }
  
    if (gk_repository_state_enabled(session->repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE)) {
        if (gk_merge_into_head(session) != GK_SUCCESS) {
            gk_session_unset_repository_state_with_callback(session, GK_REPOSITORY_STATE_SYNC_IN_PROGRESS);
            return gk_session_failure(session, purpose);
        }
    }
    
    if (gk_repository_state_disabled(session->repository, GK_REPOSITORY_STATE_HAS_CONFLICTS)) {
        if (gk_push(session, session->repository->spec.remote_name) != GK_SUCCESS) {
            gk_session_unset_repository_state_with_callback(session, GK_REPOSITORY_STATE_SYNC_IN_PROGRESS);
            return gk_session_failure(session, purpose);
        }
    }

#if DEBUG_FORCE_LONG_SYNC
    log_warn(COMP_SESSION, "DBG sleeping sync");
    for (int i = 0; i < 10; i += 1) {
        sleep(1);
        log_warn(COMP_SESSION, ".");
    }
    log_warn(COMP_SESSION, "DBG done sleeping sync");
#endif
    
    if (gk_session_unset_repository_state_with_callback(session, GK_REPOSITORY_STATE_SYNC_IN_PROGRESS) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }

    return gk_session_success(session, purpose);
}
