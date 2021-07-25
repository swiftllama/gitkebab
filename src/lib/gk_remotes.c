
#include "git2.h"

#include "gk_logging.h"
#include "gk_remotes.h"
#include "gk_session_progress.h"
#include "gk_credentials.h"
#include "gk_init.h"
#include "gk_merge.h"
#include "gk_lg2_private.h"

git_remote *prepare_remote(gk_repository *repository, const char *remote_name, const char *purpose) {    
    if (repository == NULL) {
        log_error(COMP_COMMIT, "Cannot %s, repository is NULL", purpose);
        return NULL;
    }
    if (remote_name == NULL) {
        gk_repository_failure(repository, &COMP_REMOTE, -1, "Cannot %s, remote is NULL", purpose);
        return NULL;
    }
    if (gk_repository_state_disabled(repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS)) {
        gk_repository_failure(repository, &COMP_COMMIT, -3, "Cannot %s, local checkout does not exist", purpose);
        return NULL;
    }
    else if (repository->lg2_resources->repository == NULL) {
        gk_repository_failure(repository, &COMP_COMMIT, -3, "Cannot %s, internal git2 repository is unexpectedely NULL", purpose);
        return NULL;
    }

    git_remote *remote = NULL;
    int rc = git_remote_lookup(&remote, repository->lg2_resources->repository, remote_name);
    if (rc != 0) {
        git_remote_free(remote);
        const git_error *err = git_error_last();
        gk_repository_failure(repository, &COMP_REMOTE, -3, "Cannot push, error looking up remote '%s' (%d): %s", remote_name, err->klass, err->message);
        return NULL;
    }

    return remote;
}

static int gk_repository_credential_callback(git_credential **out,
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
        result = gk_result_new(-1, "session payload is NULL");
        log_error(COMP_AUTH, gk_result_message(result));
        return -1;
    }
    gk_session *session = (gk_session *)payload;
    if (session->repository == NULL) {
        log_error(COMP_AUTH, "repository is NULL when trying to authenticate");
        return -1;
    }
    else if (session->credential == NULL) {
        result = gk_result_new(-1, "session repository has NULL credential, cannot auth" );
        log_error(COMP_AUTH, gk_result_message(result));
        gk_repository_set_last_result(session->repository, result);
        return -1;
    }

    int cred_type = session->credential->credential_type;
    if (cred_type == CREDENTIAL_SSH_KEY_MEMORY) {
        log_info(COMP_AUTH, "authenticating repository with an SSH_KEY_MEMORY credential");
        git_credential_ssh_key_memory_new((git_credential **)out, username_from_url, session->credential->ssh_public_key_bytes, session->credential->ssh_private_key_bytes, session->credential->ssh_private_key_passphrase);
    }
    else if (cred_type == CREDENTIAL_SSH_KEY_FILE) {
        log_info(COMP_AUTH, "authenticating repository with an SSH_KEY_FILE credential");
        git_credential_ssh_key_new((git_credential **)out, username_from_url, session->credential->ssh_public_key_path, session->credential->ssh_private_key_path, session->credential->ssh_private_key_passphrase);
    }
    else if (cred_type == CREDENTIAL_USERNAME_PASSWORD) {
        log_info(COMP_AUTH, "authenticating repository with a USERNAME_PASSWORD credential");
        git_credential_userpass_plaintext_new((git_credential **)out, session->credential->username, session->credential->password);
    }
    else {
        result = gk_result_new(-1, "session repository has gk_credential of unknown type");
        log_error(COMP_AUTH, "session repository has gk_credential of unknown type %d. Expected one of CREDENTIAL_SSH_KEY_MEMORY (%d), CREDENTIAL_SSH_KEY_FILE (%d) or CREDENTIAL_USERNAME_PASSWORD (%d). Cannot auth");
        gk_repository_set_last_result(session->repository, result);
        return -1;
    }
    log_info(COMP_AUTH, "Successfully set credential");
    return 0;
}

int gk_repository_clone(gk_repository *repository, gk_repository_credential *credential) {
    if ((repository == NULL)) {
        log_error(COMP_CLONE, "Cannot clone, repository is NULL");
        return GK_FAILURE;
    }
    if (gk_did_init() != 1) {
        return gk_repository_failure(repository, &COMP_CLONE, -3, "Gitkebab not initialized");
    }
        
    gk_session session;
    gk_session_init(&session, repository, credential);

    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    int rc;

    /* Set up options */
    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_opts.progress_cb = gk_repository_checkout_progress_callback;
    checkout_opts.progress_payload = &session;
    clone_opts.checkout_opts = checkout_opts;
    //clone_opts.fetch_opts.callbacks.sideband_progress = sideband_progress;
    clone_opts.fetch_opts.callbacks.transfer_progress = (git_indexer_progress_cb)&gk_repository_fetch_progress_callback;
    clone_opts.fetch_opts.callbacks.credentials = &gk_repository_credential_callback;
    clone_opts.fetch_opts.callbacks.payload = &session;

    /* Do the clone */
    log_info(COMP_CLONE, "Cloning repo");
    log_info(COMP_CLONE, "  - URL:        %s", repository->repository_spec.remote_url);
    log_info(COMP_CLONE, "  - Local path: %s", repository->repository_spec.local_path);
    gk_repository_state_set(repository, GK_REPOSITORY_STATE_CLONE_IN_PROGRESS);
    rc = git_clone((git_repository **)&(repository->lg2_resources->repository), repository->repository_spec.remote_url, repository->repository.local_path, &clone_opts);
    gk_repository_state_unset(repository, GK_REPOSITORY_STATE_CLONE_IN_PROGRESS);

    if (rc != 0) {
        git_repository_free(repository->lg2_resources->repository);
        repository->lg2_resources->repository = NULL;
        const git_error *err = git_error_last();
        return gk_repository_failure(repository, &COMP_CLONE, -1, "Clone failed (%d): %s", err->klass, err->message);
    }

    rc = git_remote_add_push(repository->lg2_resources->repository, repository->repository_spec.remote_name, repository->repository.push_refspec);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_repository_failure(repository, &COMP_CLONE, -1, "Error adding push refspec '%s' to remote '%s'  (%d): %s", repository->repository_spec.remote_name, repository->repository.push_refspec, err->klass, err->message);
    }

    gk_repository_state_set(repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS);

    log_info(COMP_CLONE, "Clone succeeded");
    return gk_repository_success(repository);
}

int gk_repository_fetch(gk_repository *repository, gk_repository_credential *credential, const char *remote_name) {
    git_remote *remote = prepare_remote(repository, remote_name, "fetch");
    if (remote == NULL) {
        return GK_FAILURE;
    }

    gk_session session;
    gk_session_init(&session, repository, credential);

    git_fetch_options fetch_options = GIT_FETCH_OPTIONS_INIT;
    fetch_options.callbacks.transfer_progress = (git_indexer_progress_cb)&gk_repository_fetch_progress_callback;
    fetch_options.callbacks.credentials = &gk_repository_credential_callback;
    fetch_options.callbacks.payload = &session;

    const git_strarray *refspecs = NULL;
    gk_repository_state_set(repository, GK_REPOSITORY_STATE_FETCH_IN_PROGRESS);
    int rc = git_remote_fetch(remote, refspecs, &fetch_options, "fetch");
    gk_repository_state_unset(repository, GK_REPOSITORY_STATE_FETCH_IN_PROGRESS);
    git_remote_free(remote);
    
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_repository_failure(repository, &COMP_REMOTE, -3, "Error fetching from remote '%s' (%d): %s", remote_name, err->klass, err->message);
    }

    rc = gk_lg2_load_references(repository);
    if (rc != 0) {
        gk_lg2_free_references(repository);
        return GK_FAILURE;
    }
    rc = gk_repository_analyze_merge_into_head(repository, repository->repository_spec.remote_ref_name, NULL);
    gk_lg2_free_references(repository);

    if (rc != 0) {
        return GK_FAILURE;
    }
    
    return gk_repository_success(repository);
}

int gk_repository_push(gk_repository *repository, gk_repository_credential *credential, const char *remote_name) {
    git_remote *remote = prepare_remote(repository, remote_name, "push");
    if (remote == NULL) {
        return GK_FAILURE;
    }

    gk_session session;
    gk_session_init(&session, repository, credential);
    
    git_push_options push_options = GIT_PUSH_OPTIONS_INIT;
    push_options.callbacks.push_transfer_progress = (git_push_transfer_progress_cb)&gk_repository_progress_push_transfer_callback;
    push_options.callbacks.credentials = gk_repository_credential_callback;
    push_options.callbacks.payload = &session;

    gk_repository_state_set(repository, GK_REPOSITORY_STATE_PUSH_IN_PROGRESS);
    int rc = git_remote_push(remote, NULL, &push_options);
    gk_repository_state_unset(repository, GK_REPOSITORY_STATE_PUSH_IN_PROGRESS);
    git_remote_free(remote);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_repository_failure(repository, &COMP_REMOTE, -3, "Error pushing to remote '%s' (%d): %s", remote_name, err->klass, err->message);
    }
    
    return gk_repository_success(repository);
}

