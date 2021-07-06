
#include "git2.h"

#include "gk_logging.h"
#include "gk_push.h"
#include "gk_session_progress.h"
#include "gk_credentials.h"

git_remote *prepare_remote(gk_session_t *session, const char *remote_name, const char *purpose) {    
    if (session == NULL) {
        log_error(COMP_COMMIT, "Cannot %s, session is NULL", purpose);
        return NULL;
    }
    if (remote_name == NULL) {
        gk_session_failure(session, &COMP_REMOTE, -1, "Cannot %s, remote is NULL", purpose);
        return NULL;
    }
    if (session->state.local_checkout_exists == 0) {
        gk_session_failure(session, &COMP_COMMIT, -3, "Cannot %s, local checkout does not exist", purpose);
        return NULL;
    }
    else if (session->lg2_repository == NULL) {
        gk_session_failure(session, &COMP_COMMIT, -3, "Cannot %s, internal git2 repository is unexpectedely NULL", purpose);
        return NULL;
    }

    git_remote *remote = NULL;
    int rc = git_remote_lookup(&remote, session->lg2_repository, remote_name);
    if (rc != 0) {
        git_remote_free(remote);
        const git_error *err = git_error_last();
        gk_session_failure(session, &COMP_REMOTE, -3, "Cannot push, error looking up remote '%s' (%d): %s", remote_name, err->klass, err->message);
        return NULL;
    }

    return remote;
}

int gk_session_fetch(gk_session_t *session, gk_session_credential_t *credential, const char *remote_name) {
    git_remote *remote = prepare_remote(session, remote_name, "fetch");
    if (remote == NULL) {
        return GK_FAILURE;
    }

    gk_authenticated_session_t authed_session;
    gk_authenticated_session_init(&authed_session, session, credential);

    git_fetch_options fetch_options = GIT_FETCH_OPTIONS_INIT;
    fetch_options.callbacks.transfer_progress = (git_indexer_progress_cb)&gk_session_fetch_progress_callback;
    fetch_options.callbacks.credentials = (git_credential_acquire_cb)&gk_session_credential_callback;
    fetch_options.callbacks.payload = &authed_session;

    const git_strarray *refspecs = NULL;
    int rc = git_remote_fetch(remote, refspecs, &fetch_options, "fetch");
    git_remote_free(remote);
    
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_REMOTE, -3, "Error fetching from remote '%s' (%d): %s", remote_name, err->klass, err->message);
    }
    
    return gk_session_success(session);
}

int gk_session_push(gk_session_t *session, gk_session_credential_t *credential, const char *remote_name) {
    git_remote *remote = prepare_remote(session, remote_name, "push");
    if (remote == NULL) {
        return GK_FAILURE;
    }

    gk_authenticated_session_t authed_session;
    gk_authenticated_session_init(&authed_session, session, credential);
    
    git_push_options push_options = GIT_PUSH_OPTIONS_INIT;
    push_options.callbacks.push_transfer_progress = (git_push_transfer_progress_cb)&gk_session_progress_push_transfer_callback;
    push_options.callbacks.credentials = (git_credential_acquire_cb)&gk_session_credential_callback;
    push_options.callbacks.payload = &authed_session;

    int rc = git_remote_push(remote, NULL, &push_options);
    git_remote_free(remote);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_REMOTE, -3, "Error pushing to remote '%s' (%d): %s", remote_name, err->klass, err->message);
    }
    
    return gk_session_success(session);
}
