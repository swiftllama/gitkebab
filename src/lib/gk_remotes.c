
#include <stdio.h>
#include <string.h>

#include "git2.h"

#include "gk_logging.h"
#include "gk_remotes.h"
#include "gk_session_progress.h"
#include "gk_credentials.h"
#include "gk_init.h"
#include "gk_merge.h"
#include "gk_lg2_private.h"
#include "gk_session.h"
#include "gk_filesystem.h"

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
    if (gk_session_context_push(session, purpose, &COMP_AUTH, GK_REPOSITORY_VERIFY_INITIALIZED) != GK_SUCCESS) {
        return -1;
    }
    if (session->repository == NULL) {
        gk_session_failure_ex(session, purpose, GK_ERR, "repository is NULL when trying to authenticate");
        return -1;
    }

    int cred_type = session->credential.credential_type;
    const char *username = username_from_url != NULL ? username_from_url : session->credential.username;
    if (cred_type == CREDENTIAL_SSH_KEY_MEMORY) {
        if ((allowed_ssh_memory == 0) && (allowed_username >= 1)) {
            log_info(COMP_AUTH, "preauthenticating repository with username for SSH_KEY_MEMORY credential");
            int rc = git_credential_username_new((git_credential **) out, username);
            if (rc != 0) {
                gk_session_lg2_failure_ex(session, purpose, rc, "error creating username credential for user [%s]", username);
                return -1;
            }
        }
        else {
            log_info(COMP_AUTH, "authenticating repository with an SSH_KEY_MEMORY credential with username [%s]", username);
            if (session->credential.ssh_private_key_bytes == NULL) {
                gk_session_lg2_failure_ex(session, purpose, GK_ERR, "session has an in-memory SSH credential for authentication but the private key bytes are NULL");
                return -1;
            }
            if (username == NULL) {
                gk_session_lg2_failure_ex(session, purpose, GK_ERR, "session has an in-memory SSH credential for authentication but the username is NULL");
                return -1;
            }
            int rc = git_credential_ssh_key_memory_new((git_credential **)out, username, session->credential.ssh_public_key_bytes, session->credential.ssh_private_key_bytes, session->credential.ssh_private_key_passphrase);
            if (rc != 0) {
                gk_session_lg2_failure_ex(session, purpose, rc, "error creating ssh key memory credential for user [%s]", username);
                return -1;
            }
        }
    }
    else if (cred_type == CREDENTIAL_SSH_KEY_FILE) {
        if ((allowed_ssh_key == 0) && (allowed_username >= 1)) {
            log_info(COMP_AUTH, "preauthenticating repository with username for SSH_KEY_FILE credential");
            int rc = git_credential_username_new((git_credential **) out, username);
            if (rc != 0) {
                gk_session_lg2_failure_ex(session, purpose, rc, "error creating ssh key file credential for user [%s]", username);
                return -1;
            }
        }
        else {
            log_info(COMP_AUTH, "authenticating repository with an SSH_KEY_FILE credential");
            git_credential_ssh_key_new((git_credential **)out, username, session->credential.ssh_public_key_path, session->credential.ssh_private_key_path, session->credential.ssh_private_key_passphrase);
        }
    }
    else if (cred_type == CREDENTIAL_USERNAME_PASSWORD) {
        log_info(COMP_AUTH, "authenticating repository with a USERNAME_PASSWORD credential");
        int rc = git_credential_userpass_plaintext_new((git_credential **)out, session->credential.username, session->credential.password);
        if (rc != 0) {
            gk_session_lg2_failure_ex(session, purpose, rc, "error creating user pass credential for user [%s]", username);
            return -1;
        }
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
    if (gk_session_context_push(session, purpose, &COMP_CLONE, GK_REPOSITORY_VERIFY_INITIALIZED) != GK_SUCCESS) {
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


    // Verify source
    const char *source_url = session->repository->spec.source_url;
    if (session->repository->spec.source_url_type == GK_REPOSITORY_SOURCE_URL_FILESYSTEM) {
        // verify that the path exists
        if (gk_directory_exists(source_url) == 0) {
            return gk_session_failure_ex(session, purpose, GK_ERR_CLONE_INEXISTENT_SOURCE_PATH, "repository source path [%s] does not exist", source_url);
        }
    }

    // Verify dest
    const char *local_path = session->repository->spec.local_path;
    if ((local_path == NULL) || (local_path[0] == '\0')) {
        return gk_session_failure_ex(session, purpose, GK_ERR_CLONE_INVALID_DESTINATION_PATH, "repository destination path [%s] is invalid", local_path == NULL ? "NULL" : "");
    }
    else if ((gk_directory_exists(local_path) == 1) && (gk_directory_is_empty(local_path) == 0)) {
        return gk_session_failure_ex(session, purpose, GK_ERR_CLONE_DESTINATION_PATH_NONEMPTY, "repository destination path [%s] exists but is not empty", local_path);
    }
    
    /* Do the clone */
    log_info(COMP_CLONE, "Cloning repo");
    log_info(COMP_CLONE, "  - URL:        [%s]", source_url);
    log_info(COMP_CLONE, "  - URL type: [%d]", session->repository->spec.source_url_type);
    log_info(COMP_CLONE, "  - Local path: [%s]", local_path);
    log_info(COMP_CLONE, "  - Credential: %s", gk_credential_description(&session->credential));
    gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_CLONE_IN_PROGRESS);
    if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }
    rc = git_clone((git_repository **)&(session->repository->lg2_resources->repository), session->repository->spec.source_url, session->repository->spec.local_path, &clone_opts);
    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_CLONE_IN_PROGRESS);
    if (rc != 0) {
        if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
            return gk_session_failure(session, purpose);
        }
        int return_value = gk_session_lg2_failure(session, purpose, GK_ERR);
        git_repository_free(session->repository->lg2_resources->repository);
        session->repository->lg2_resources->repository = NULL;
        return return_value;
    }

    
    // Set HEAD to main branch
    char ref_name[128];
    snprintf(ref_name, 128, "refs/heads/%s", session->repository->spec.main_branch_name);
    if (git_repository_set_head(session->repository->lg2_resources->repository, ref_name) != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to set HEAD to [%s]", ref_name);
    }

    /*
    // If there are no commits, create one
    git_reference *head_ref = NULL;
    int rc = git_repository_head(&head_ref, session->repository->lg2_resources->repository);
    if (rc == GIT_EUNBORNBRANCH) {
        git_oid commit_oid;
        git_signature(
        if (git_commit_create_v(&commit_oid, session->repository->lg2_resources->repository, "HEAD", lg2_resources->signature, lg2_resources->signature, NULL, safe_commit_message, lg2_resources->tree, lg2_resources->repository_head_object != NULL? 1 : 0, lg2_resources->repository_head_object) != 0) {
        gk_lg2_free_references(session->repository);
        gk_lg2_signature_free(session->repository);
        gk_lg2_tree_free(session->repository);
        gk_lg2_index_free(session->repository);
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to create commit");
    }
    }
    else if (rc != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed parsing HEAD to verify repository isn't empty", );
    }
    git_reference_free(head_ref);
    head_ref = NULL;*/
    
    // Add remote push refspec
    if (git_remote_add_push(session->repository->lg2_resources->repository, session->repository->spec.remote_name, session->repository->spec.push_refspec) != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to add push refspec [%s] to remote [%s]", session->repository->spec.push_refspec, session->repository->spec.remote_name);
    }

    gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS);
    if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }

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
    if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }
    int rc = git_remote_fetch(remote, refspecs, &fetch_options, "fetch");
    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_FETCH_IN_PROGRESS);
    git_remote_free(remote);
    
    if (rc != 0) {
        if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
            return gk_session_failure(session, purpose);
        }
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to fetch from remote [%s]", remote_name);
    }

    if (gk_lg2_load_references(session) != 0) {
        gk_lg2_free_references(session->repository);
        if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
            return gk_session_failure(session, purpose);
        }
        return gk_session_failure(session, purpose);
    }
    rc = gk_analyze_merge_into_head(session, session->repository->spec.remote_ref_name, NULL);

    gk_lg2_free_references(session->repository);
    if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }

    if (rc != 0) {
        return gk_session_failure(session, purpose);
    }
    
    return gk_session_success(session, purpose);
}

int gk_push(gk_session *session, const char *remote_name) {
    const char *purpose = "push to remote";
    if (gk_session_context_push(session, purpose, &COMP_REMOTE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    if (gk_lg2_load_references(session) != 0) {
        gk_lg2_free_references(session->repository);
        if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
            return gk_session_failure(session, purpose);
        }
        return gk_session_failure(session, purpose);
    }

    if (session->repository->lg2_resources->repository_head_object == NULL) {
        // if the repository head object is null then there is nothing
        // to push
        return gk_session_success(session, purpose);
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

    if (session->repository->lg2_resources->fetch_head_object == NULL) {
        // if the repository fetch head is null the remote must be
        // empty, so create a head for it

        /*
        git_reference *remote_head = NULL;
	if (git_reference_symbolic_create(&remote_head, session->repository->lg2_resources->repository, "refs/remotes/origin/HEAD", "master", true, "set remote HEAD since remote repository is empty (gitkebab)") != 0) {
            return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "error setting remote HEAD to track branch [%s]", session->repository->spec.main_branch_name);
            }*/

        /*
        if (update_remote_head(session->repository->lg2_resources->repsitory, remote, session->repository->spec.main_branch_name, "set remote HEAD since remote repository is empty (gitkebab)") != 0) {
            return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "error setting remote HEAD to track branch [%s]", session->repository->spec.main_branch_name);
            }*/

        // Then reload the references
        //git_reference_free(remote_head);
        /*
        gk_lg2_free_all_but_repository(session->repository);
        if (gk_lg2_load_references(session) != GK_SUCCESS) {
            return gk_session_failure(session, purpose);
            }*/
    }

    git_push_options push_options = GIT_PUSH_OPTIONS_INIT;
    push_options.callbacks.push_transfer_progress = (git_push_transfer_progress_cb)&gk_session_progress_push_transfer_callback;
    push_options.callbacks.push_update_reference = (git_push_update_reference_cb)&gk_session_progress_push_update_reference_callback;
    push_options.callbacks.sideband_progress = (git_transport_message_cb)&gk_session_transport_message_callback;
    
    push_options.callbacks.credentials = gk_session_credential_callback;
    push_options.callbacks.payload = session;
    

    gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_PUSH_IN_PROGRESS);
    if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }
    int rc = git_remote_push(remote, NULL, &push_options);
    gk_repository_state_unset(session->repository, GK_REPOSITORY_STATE_PUSH_IN_PROGRESS);
    if (gk_session_trigger_repository_state_callback(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }

    git_remote_free(remote);
    if (rc != 0) {
        return gk_session_lg2_failure(session, purpose, GK_ERR);
    }
    
    return gk_session_success(session, purpose);
}


gk_remote_list *gk_remote_list_new(size_t length) {
    gk_remote_list *list = (gk_remote_list *)malloc(sizeof(gk_remote_list));
    if (list == NULL) return NULL;
    list->count = length;
    list->remotes = (gk_remote **)calloc(length, sizeof(gk_remote));
    if (list->remotes == NULL) {
        free(list);
        return NULL;
    }
    return list;
}

void gk_remote_list_free(gk_remote_list *list) {
    if (list == NULL) return;
    if (list->remotes != NULL) {
        free(list->remotes);
        list->remotes = NULL;
    }
    free(list);
}

gk_remote *gk_remote_new(const char *name, const char *url) {
    gk_remote *remote = (gk_remote *)malloc(sizeof(gk_remote));
    if (remote == NULL) return NULL;
    remote->name = name == NULL ? strdup("") : strdup(name);
    remote->url = url == NULL ? strdup("") : strdup(url);
    return remote;
}

void gk_remote_free(gk_remote *remote) {
    free((void *)remote->name);
    free((void *)remote->url);
    remote->name = NULL;
    remote->url = NULL;
    free(remote);
}

gk_remote_list *gk_remotes_list(gk_session *session) {
    const char *purpose = "remotes count";
    if (gk_session_context_push(session, purpose, &COMP_REMOTE, GK_REPOSITORY_VERIFY_REPOSITORY_LOADED) != GK_SUCCESS) {
        return NULL;
    }

    git_strarray remote_names;
    int rc = git_remote_list(&remote_names, session->repository->lg2_resources->repository);
    if (rc != 0) {
        gk_session_lg2_failure(session, purpose, GK_ERR);
        return NULL;
    }
    
    gk_remote_list *remote_list = gk_remote_list_new(remote_names.count);

    for (size_t i = 0; i < remote_list->count; i += 1) {
        git_remote *next_remote = NULL;
        const char *next_url = "";
        if (git_remote_lookup(&next_remote, session->repository->lg2_resources->repository, remote_names.strings[i]) == 0) {
            next_url = git_remote_url(next_remote);
        }
        remote_list->remotes[i] = gk_remote_new(remote_names.strings[i], next_url);
        if (next_remote != NULL) {
            git_remote_free(next_remote);
            next_remote = NULL;
        }
    }

    git_strarray_dispose(&remote_names);
    
    gk_session_success(session, purpose);

    return remote_list;
}


int gk_remote_delete(gk_session *session, const char* name) {
    const char *purpose = "remote destroy";
    if (gk_session_context_push(session, purpose, &COMP_REMOTE, GK_REPOSITORY_VERIFY_INITIALIZED) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    int rc = git_remote_delete(session->repository->lg2_resources->repository, name);
    if (rc != 0) {
        return gk_session_lg2_failure(session, purpose, GK_ERR);
    }

    return gk_session_success(session, purpose);
}

int gk_remote_create(gk_session *session, const char* name, const char* url) {
    const char *purpose = "remote create";
    if (gk_session_context_push(session, purpose, &COMP_REMOTE, GK_REPOSITORY_VERIFY_INITIALIZED) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    git_remote *remote = NULL;
    log_info(COMP_REMOTE, "Creating remote [%s] with url [%s]", name, url);
    int rc = git_remote_create(&remote, session->repository->lg2_resources->repository, name, url);
    if (rc != 0) {
        return gk_session_lg2_failure(session, purpose, GK_ERR);
    }

    if (git_remote_add_push(session->repository->lg2_resources->repository, name, session->repository->spec.push_refspec) != 0) {
        git_remote_free(remote);
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to add push refspec [%s] after creating remote [%s]", session->repository->spec.push_refspec, name);
    }
    
    git_remote_free(remote);

    return gk_session_success(session, purpose);
}





