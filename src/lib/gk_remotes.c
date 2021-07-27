
#include "git2.h"

#include "gk_logging.h"
#include "gk_remotes.h"
#include "gk_session_progress.h"
#include "gk_credentials.h"
#include "gk_init.h"
#include "gk_merge.h"
#include "gk_lg2_private.h"
#include "gk_session.h"

static int gk_session_credential_callback(git_credential **out,
                                   const char *url,
                                   const char *username_from_url,
                                   unsigned int allowed_types,
                                   void *payload) {
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
        log_error(COMP_AUTH, "session payload is NULL");
        return -1;
    }
    gk_session *session = (gk_session *)payload;


    const char *purpose = "authenticate session";
    if (gk_session_context_push(session, purpose, &COMP_AUTH, GK_REPOSITORY_VERIFY_DEFAULT) != GK_SUCCESS) {
        return -1;
    }
    if (session->repository == NULL) {
        gk_session_failure_ex(session, purpose, GK_ERR, "repository is NULL when trying to authenticate");
        return -1;
    }

    int cred_type = session->credential.credential_type;
    if (cred_type == CREDENTIAL_SSH_KEY_MEMORY) {
        log_info(COMP_AUTH, "authenticating repository with an SSH_KEY_MEMORY credential");
        git_credential_ssh_key_memory_new((git_credential **)out, username_from_url, session->credential.ssh_public_key_bytes, session->credential.ssh_private_key_bytes, session->credential.ssh_private_key_passphrase);
    }
    else if (cred_type == CREDENTIAL_SSH_KEY_FILE) {
        log_info(COMP_AUTH, "authenticating repository with an SSH_KEY_FILE credential");
        git_credential_ssh_key_new((git_credential **)out, username_from_url, session->credential.ssh_public_key_path, session->credential.ssh_private_key_path, session->credential.ssh_private_key_passphrase);
    }
    else if (cred_type == CREDENTIAL_USERNAME_PASSWORD) {
        log_info(COMP_AUTH, "authenticating repository with a USERNAME_PASSWORD credential");
        git_credential_userpass_plaintext_new((git_credential **)out, session->credential.username, session->credential.password);
    }
    else {
        gk_session_failure_ex(session, purpose, GK_ERR, "session repository has gk_credential of unknown type %d. Expected one of CREDENTIAL_SSH_KEY_MEMORY (%d), CREDENTIAL_SSH_KEY_FILE (%d) or CREDENTIAL_USERNAME_PASSWORD (%d). Cannot auth", cred_type, CREDENTIAL_SSH_KEY_MEMORY, CREDENTIAL_SSH_KEY_FILE, CREDENTIAL_USERNAME_PASSWORD);
        return -1;
    }
    log_info(COMP_AUTH, "Successfully set credential");
    gk_session_success(session, purpose);
    return 0;
}

int gk_clone(gk_session *session) {
    const char *purpose = "clone repository";
    if (gk_session_context_push(session, purpose, &COMP_CLONE, GK_REPOSITORY_VERIFY_DEFAULT) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    int rc;
    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    
    /* Set up options */
    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_opts.progress_cb = gk_session_checkout_progress_callback;
    checkout_opts.progress_payload = session;
    clone_opts.checkout_opts = checkout_opts;
    //clone_opts.fetch_opts.callbacks.sideband_progress = sideband_progress;
    clone_opts.fetch_opts.callbacks.transfer_progress = (git_indexer_progress_cb)&gk_session_fetch_progress_callback;
    clone_opts.fetch_opts.callbacks.credentials = &gk_session_credential_callback;
    clone_opts.fetch_opts.callbacks.payload = session;

    /* Do the clone */
    log_info(COMP_CLONE, "Cloning repo");
    log_info(COMP_CLONE, "  - URL:        [%s]", session->repository->repository_spec.remote_url);
    log_info(COMP_CLONE, "  - Local path: [%s]", session->repository->repository_spec.local_path);
    gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_CLONE_IN_PROGRESS);
    rc = git_clone((git_repository **)&(session->repository->lg2_resources->repository), session->repository->repository_spec.remote_url, session->repository->repository_spec.local_path, &clone_opts);
    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_CLONE_IN_PROGRESS);

    if (rc != 0) {
        git_repository_free(session->repository->lg2_resources->repository);
        session->repository->lg2_resources->repository = NULL;
        return gk_session_lg2_failure(session, purpose, GK_ERR);
    }

    if (git_remote_add_push(session->repository->lg2_resources->repository, session->repository->repository_spec.remote_name, session->repository->repository_spec.push_refspec) != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to add push refspec [%s] to remote [%s]", session->repository->repository_spec.push_refspec, session->repository->repository_spec.remote_name);
    }

    gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS);

    log_info(COMP_CLONE, "Clone succeeded");
    return gk_session_success(session, purpose);
}

int gk_fetch(gk_session *session, const char *remote_name) {
    const char *purpose = "fetch from remote";
    if (gk_session_context_push(session, purpose, &COMP_REMOTE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    if (remote_name == NULL) {
        return gk_session_failure_ex(session, purpose, GK_ERR, "remote is NULL");
    }
    git_remote *remote = NULL;
    if (git_remote_lookup(&remote, lg2_resources->repository, remote_name) != 0) {
        git_remote_free(remote);
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "error looking up remote [%s]", remote_name);
    }
    
    git_fetch_options fetch_options = GIT_FETCH_OPTIONS_INIT;
    fetch_options.callbacks.transfer_progress = (git_indexer_progress_cb)&gk_session_fetch_progress_callback;
    fetch_options.callbacks.credentials = &gk_session_credential_callback;
    fetch_options.callbacks.payload = session;

    const git_strarray *refspecs = NULL;
    gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_FETCH_IN_PROGRESS);
    int rc = git_remote_fetch(remote, refspecs, &fetch_options, "fetch");
    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_FETCH_IN_PROGRESS);
    git_remote_free(remote);
    
    if (rc != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to fetch from remote [%s]", remote_name);
    }

    if (gk_lg2_load_references(session) != 0) {
        gk_lg2_free_references(session->repository);
        return gk_session_failure(session);
    }
    rc = gk_analyze_merge_into_head(session, session->repository->repository_spec.remote_ref_name, NULL);
    gk_lg2_free_references(session->repository);

    if (rc != 0) {
        return gk_session_failure(session);
    }
    
    return gk_session_success(session, purpose);
}

int gk_push(gk_session *session, const char *remote_name) {
    const char *purpose = "push to remote";
    if (gk_session_context_push(session, purpose, &COMP_REMOTE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    if (remote_name == NULL) {
        return gk_session_failure_ex(session, purpose, GK_ERR, "remote is NULL");
    }
    git_remote *remote = NULL;
    if (git_remote_lookup(&remote, lg2_resources->repository, remote_name) != 0) {
        git_remote_free(remote);
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "error looking up remote [%s]", remote_name);
    }
    
    git_push_options push_options = GIT_PUSH_OPTIONS_INIT;
    push_options.callbacks.push_transfer_progress = (git_push_transfer_progress_cb)&gk_session_progress_push_transfer_callback;
    push_options.callbacks.credentials = gk_session_credential_callback;
    push_options.callbacks.payload = session;

    gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_PUSH_IN_PROGRESS);
    int rc = git_remote_push(remote, NULL, &push_options);
    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_PUSH_IN_PROGRESS);
    git_remote_free(remote);
    if (rc != 0) {
        return gk_session_lg2_failure(session, purpose, GK_ERR);
    }
    
    return gk_session_success(session, purpose);
}

