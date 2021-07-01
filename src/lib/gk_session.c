
#include "stdio.h"

#include "gk_session.h"
#include "gk_results.h"
#include "gk_logging.h"
#include "git2.h"
#include "gitkebab.h"

void gk_repository_init(gk_repository_t *repository, const char *remote_url, const char *local_path, const char *usr) {
    if (repository == NULL) {
        return;
    }
    repository->local_path = local_path;
    repository->remote_url = remote_url;
    repository->user = usr;
}

void gk_authenticated_session_init(gk_authenticated_session_t *authed_session, gk_session_t *session, gk_session_credential_t *credential) {
    if (authed_session == NULL) {
        return;
    }
    authed_session->session = session;
    authed_session->credential = credential;
}

void gk_session_init(gk_session_t *session, gk_repository_t *repository, gk_session_progress_callback_t *progress_callback) {
    if (session == NULL) {
        return;
    }
    session->repository = repository;
    session->last_result = gk_result_success();
    session->callbacks.progress_callback = progress_callback;
}

void gk_session_set_last_result(gk_session_t *session, gk_result_t *last_result) {
    gk_result_free(session->last_result);
    session->last_result = last_result;
    if (last_result == NULL) {
        log_error(COMP_GENERAL, "Set a NULL last error on a session, this could be the result of a memory allocation failure");
    }
}

int gk_session_credential_callback(git_credential **out,
                                   const char *url,
                                   const char *username_from_url,
                                   unsigned int allowed_types,
                                   void *payload) {
    gk_result_t *result = NULL;
    
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
        result = gk_result(-1, "authed-session payload is NULL");
        log_error(COMP_AUTH, gk_result_message(result));
        return -1;
    }
    gk_authenticated_session_t *authed_session = (gk_authenticated_session_t *)payload;
    if (authed_session->session == NULL) {
        log_error(COMP_AUTH, "session is NULL when trying to authenticate");
        return -1;
    }
    else if (authed_session->credential == NULL) {
        result = gk_result(-1, "authed session has NULL credential, cannot auth" );
        log_error(COMP_AUTH, gk_result_message(result));
        gk_session_set_last_result(authed_session->session, result);
        return -1;
    }

    int cred_type = authed_session->credential->credential_type;
    if (cred_type == CREDENTIAL_SSH_KEY_MEMORY) {
        log_info(COMP_AUTH, "authenticating session with an SSH_KEY_MEMORY credential");
        git_credential_ssh_key_memory_new(out, username_from_url, authed_session->credential->ssh_public_key_bytes, authed_session->credential->ssh_private_key_bytes, authed_session->credential->ssh_private_key_passphrase);
    }
    else if (cred_type == CREDENTIAL_SSH_KEY_FILE) {
        log_info(COMP_AUTH, "authenticating session with an SSH_KEY_FILE credential");
        git_credential_ssh_key_new(out, username_from_url, authed_session->credential->ssh_public_key_path, authed_session->credential->ssh_private_key_path, authed_session->credential->ssh_private_key_passphrase);
    }
    else if (cred_type == CREDENTIAL_USERNAME_PASSWORD) {
        log_info(COMP_AUTH, "authenticating session with a USERNAME_PASSWORD credential");
        git_credential_userpass_plaintext_new(out, authed_session->credential->username, authed_session->credential->password);
    }
    else {
        result = gk_result(-1, "authed session has gk_credential of unknown type");
        log_error(COMP_AUTH, "authed session has gk_credential of unknown type %d. Expected one of CREDENTIAL_SSH_KEY_MEMORY (%d), CREDENTIAL_SSH_KEY_FILE (%d) or CREDENTIAL_USERNAME_PASSWORD (%d). Cannot auth");
        gk_session_set_last_result(authed_session->session, result);
        return -1;
    }
    log_info(COMP_AUTH, "Successfully set credential");
    return 0;
}

void gk_session_clone(gk_session_t *session, gk_session_credential_t *credential) {
    gk_result_t *result = NULL;
    
    if ((session == NULL)) {
        result = gk_result(-1, "Cannot clone, session is NULL");
        log_error(COMP_CLONE, gk_result_message(result));
        gk_session_set_last_result(session, result);
        return;
    }
    else if (session->repository == NULL) {
        result = gk_result(-2, "Cannot clone, session respository is NULL");
        log_error(COMP_CLONE, gk_result_message(result));
        gk_session_set_last_result(session, result);
        return;
    }
    if (gk_did_init() != 1) {
        result = gk_result(-3, "Gitkebab not initialized");
        log_error(COMP_CLONE, gk_result_message(result));
        gk_session_set_last_result(session, result);
        return;
    }
        
    gk_authenticated_session_t authed_session;
    gk_authenticated_session_init(&authed_session, session, credential);

    git_repository *cloned_repo = NULL;
    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    int error;

    /* Set up options */
    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_opts.progress_cb = gk_session_checkout_progress_callback;
    checkout_opts.progress_payload = &authed_session;
    clone_opts.checkout_opts = checkout_opts;
    //clone_opts.fetch_opts.callbacks.sideband_progress = sideband_progress;
    clone_opts.fetch_opts.callbacks.transfer_progress = (git_indexer_progress_cb)&gk_session_fetch_progress_callback;
    clone_opts.fetch_opts.callbacks.credentials = &gk_session_credential_callback;
    clone_opts.fetch_opts.callbacks.payload = &authed_session;

    /* Do the clone */
    log_info(COMP_CLONE, "Cloning repo");
    log_info(COMP_CLONE, "  - URL:        %s", session->repository->remote_url);
    log_info(COMP_CLONE, "  - Local path: %s", session->repository->local_path);
    error = git_clone(&cloned_repo, session->repository->remote_url, session->repository->local_path, &clone_opts);

    if (error != 0) {
        const git_error *err = git_error_last();
        if (err) {
            log_error(COMP_CLONE, "Clone failed: %s", err->message);
            gk_session_set_last_result(session, gk_result(err->klass, err->message));
        }
        else {
            log_error(COMP_CLONE, "Clone failed with git error code %d", error);
            gk_session_set_last_result(session, gk_result(error, ""));
        }
    }
    else {
        log_info(COMP_CLONE, "Clone succeeded");
        gk_session_set_last_result(session, gk_result_success());   
    }
    if (cloned_repo) {
        git_repository_free(cloned_repo);
    }
}
