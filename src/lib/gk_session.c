
#include "stdio.h"
#include <string.h>

#include "git2.h"
#include "gitkebab.h"
#include "gk_session.h"
#include "gk_results.h"
#include "gk_logging.h"
#include "gk_status.h"

static void gk_repository_init(gk_repository *repository, const char *remote_url, const char *local_path, const char *usr) {
    if (repository == NULL) {
        return;
    }
    if (remote_url == NULL) {
        log_warn(COMP_GENERAL, "Repository initialized with NULL remote_url, will use empty string instead");
    }
    if (local_path == NULL) {
        log_warn(COMP_GENERAL, "Repository initialized with NULL local_url, will use empty string instead");
    }
    if (usr == NULL) {
        log_warn(COMP_GENERAL, "Repository initialized with NULL user, will use empty string instead");
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

void gk_session_init(gk_session *session, const char *remote_url, const char *local_path, const char *user, gk_session_progress_callback *progress_callback) {
    if (session == NULL) {
        return;
    }
    gk_repository_init(&session->repository, remote_url, local_path, user);
    session->last_result = gk_result_success();
    session->callbacks.progress_callback = progress_callback;

    session->state.local_checkout_exists = 0;
    session->state.has_conflicts = 0;
    session->state.merge_in_progress = 0;
    session->state.clone_in_progress = 0;
    session->state.push_in_progress = 0;
    session->state.pull_in_progress = 0;

    gk_status_summary_reset(&session->status_summary);
    
    session->lg2_repository = NULL;
    session->lg2_status_list = NULL;
}

int gk_session_open_local_repository(gk_session *session) {
    gk_result *result = NULL;
    
    if (session == NULL) {
        log_error(COMP_GENERAL, "gk_session_open_local_repository(session) called on NULL session");
        return GK_FAILURE;
    }

    int rc = git_repository_open((git_repository **)&(session->lg2_repository), session->repository.local_path);
    if (rc != 0) {
        git_repository_free(session->lg2_repository);
        session->lg2_repository = NULL;
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_GENERAL, -2, "Error opening repository at local path (%d): %s", err->klass, err->message);
    }
    
    session->state.local_checkout_exists = 1;
    return gk_session_success(session);
}

void gk_session_set_last_result(gk_session *session, gk_result *last_result) {
    gk_result_free(session->last_result);
    session->last_result = last_result;
    if (last_result == NULL) {
        log_error(COMP_GENERAL, "Set a NULL last error on a session, this could be the result of a memory allocation failure");
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

int gk_session_credential_callback(void **out,
                                   const char *url,
                                   const char *username_from_url,
                                   unsigned int allowed_types,
                                   void *payload) {
    gk_result *result = NULL;
    
    int allowed_userpass_plaintext = allowed_types & GIT_CREDENTIAL_USERPASS_PLAINTEXT;
    int allowed_ssh_key = allowed_types & GIT_CREDENTIAL_SSH_KEY;
    int allowed_ssh_custom = allowed_types & GIT_CREDENTIAL_SSH_CUSTOM;
    int allowed_default = allowed_types & GIT_CREDENTIAL_DEFAULT;
    int allowed_ssh_interactive = allowed_types & GIT_CREDENTIAL_SSH_INTERACTIVE;
    int allowed_username = allowed_types & GIT_CREDENTIAL_USERNAME;
    int allowed_ssh_memory = allowed_types & GIT_CREDENTIAL_SSH_MEMORY;
    
    
    log_info(COMP_AUTH, "credential callback called for url [%s], username [%s], allowed_types [%d = userpass_plaintext:%s, ssh_key: %s, ssh_custom: %s, default: %s, ssh_interactive: %s, username: %s, ssh_memory: %s]",
             url,
             username_from_url,
             allowed_types,
             allowed_userpass_plaintext > 0 ? "on" : "off",
             allowed_ssh_key > 0 ? "on" : "off",
             allowed_ssh_custom > 0 ? "on" : "off",
             allowed_default > 0 ? "on" : "off",
             allowed_ssh_interactive > 0 ? "on" : "off",
             allowed_username > 0 ? "on" : "off",
             allowed_ssh_memory > 0 ? "on" : "off");
    
    if (payload == NULL) {
        result = gk_result_new(-1, "authed-session payload is NULL");
        log_error(COMP_AUTH, gk_result_message(result));
        return -1;
    }
    gk_authenticated_session *authed_session = (gk_authenticated_session *)payload;
    if (authed_session->session == NULL) {
        log_error(COMP_AUTH, "session is NULL when trying to authenticate");
        return -1;
    }
    else if (authed_session->credential == NULL) {
        result = gk_result_new(-1, "authed session has NULL credential, cannot auth" );
        log_error(COMP_AUTH, gk_result_message(result));
        gk_session_set_last_result(authed_session->session, result);
        return -1;
    }

    int cred_type = authed_session->credential->credential_type;
    if (cred_type == CREDENTIAL_SSH_KEY_MEMORY) {
        log_info(COMP_AUTH, "authenticating session with an SSH_KEY_MEMORY credential");
        git_credential_ssh_key_memory_new((git_credential **)out, username_from_url, authed_session->credential->ssh_public_key_bytes, authed_session->credential->ssh_private_key_bytes, authed_session->credential->ssh_private_key_passphrase);
    }
    else if (cred_type == CREDENTIAL_SSH_KEY_FILE) {
        log_info(COMP_AUTH, "authenticating session with an SSH_KEY_FILE credential");
        git_credential_ssh_key_new((git_credential **)out, username_from_url, authed_session->credential->ssh_public_key_path, authed_session->credential->ssh_private_key_path, authed_session->credential->ssh_private_key_passphrase);
    }
    else if (cred_type == CREDENTIAL_USERNAME_PASSWORD) {
        log_info(COMP_AUTH, "authenticating session with a USERNAME_PASSWORD credential");
        git_credential_userpass_plaintext_new((git_credential **)out, authed_session->credential->username, authed_session->credential->password);
    }
    else {
        result = gk_result_new(-1, "authed session has gk_credential of unknown type");
        log_error(COMP_AUTH, "authed session has gk_credential of unknown type %d. Expected one of CREDENTIAL_SSH_KEY_MEMORY (%d), CREDENTIAL_SSH_KEY_FILE (%d) or CREDENTIAL_USERNAME_PASSWORD (%d). Cannot auth");
        gk_session_set_last_result(authed_session->session, result);
        return -1;
    }
    log_info(COMP_AUTH, "Successfully set credential");
    return 0;
}

int gk_session_clone(gk_session *session, gk_session_credential *credential) {
    gk_result *result = NULL;
    
    if ((session == NULL)) {
        log_error(COMP_CLONE, "Cannot clone, session is NULL");
        return GK_FAILURE;
    }
    if (gk_did_init() != 1) {
        return gk_session_failure(session, &COMP_CLONE, -3, "Gitkebab not initialized");
    }
        
    gk_authenticated_session authed_session;
    gk_authenticated_session_init(&authed_session, session, credential);

    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    int rc;

    /* Set up options */
    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_opts.progress_cb = gk_session_checkout_progress_callback;
    checkout_opts.progress_payload = &authed_session;
    clone_opts.checkout_opts = checkout_opts;
    //clone_opts.fetch_opts.callbacks.sideband_progress = sideband_progress;
    clone_opts.fetch_opts.callbacks.transfer_progress = (git_indexer_progress_cb)&gk_session_fetch_progress_callback;
    clone_opts.fetch_opts.callbacks.credentials = (git_credential_acquire_cb)&gk_session_credential_callback;
    clone_opts.fetch_opts.callbacks.payload = &authed_session;

    /* Do the clone */
    log_info(COMP_CLONE, "Cloning repo");
    log_info(COMP_CLONE, "  - URL:        %s", session->repository.remote_url);
    log_info(COMP_CLONE, "  - Local path: %s", session->repository.local_path);
    rc = git_clone((git_repository **)&(session->lg2_repository), session->repository.remote_url, session->repository.local_path, &clone_opts);

    if (rc != 0) {
        git_repository_free(session->lg2_repository);
        session->lg2_repository = NULL;
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CLONE, -1, "Clone failed (%d): %s", err->klass, err->message);
    }

    rc = git_remote_add_push(session->lg2_repository, "origin", "refs/heads/master:refs/heads/master");
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CLONE, -1, "Error adding push refspec to remote 'origin'  (%d): %s", err->klass, err->message);
    }

    session->state.local_checkout_exists = 1;

    log_info(COMP_CLONE, "Clone succeeded");
    return gk_session_success(session);
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
