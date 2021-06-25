
#include "stdio.h"
#include "gk_session.h"
#include "results.h"

log_Component COMP_CLONE = {LOG_DEBUG, "CLONE"};

void gk_repository_init(gk_repository_t *repository, const char *remote_url, const char *local_path, const char *usr) {
    if (repository == NULL) {
        return;
    }
    repository->local_path = local_path;
    repository->remote_url = remote_url;
    repository->user = usr;
}

void gk_authenticated_session_init(gk_authenticated_session_t *authed_session, gk_session_t *session, git_credential *credential) {
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
    //session->credential = NULL;
    session->last_result = gk_result_success();
    session->callbacks.progress_callback = progress_callback;
}

void gk_session_set_last_result(gk_session_t *session, gk_result_t last_result) {
    gk_result_free(&(session->last_result));
    session->last_result = last_result;
}


int gk_session_credential_callback(git_credential **out,
                                   const char *url,
                                   const char *username_from_url,
                                   unsigned int allowed_types,
                                   void *payload) {
    if (payload == NULL) {
        printf("ERROR: Expected authed-session payload but found NULL");
        return -1;
    }
    gk_authenticated_session_t *authed_session = (gk_authenticated_session_t *)payload;
    if (authed_session->credential == NULL) {
        printf("ERROR: authed session has NULL credential, cannot auth");
        return -1;
    }
    *out = authed_session->credential;
    return 0;
}

static int gk_session_fetch_progress_callback(const git_indexer_progress *stats, void *payload)
{
    /*
	progress_data *pd = (progress_data*)payload;
	pd->fetch_progress = *stats;
	print_progress(pd);*/
    printf("FETCH PROGRESS\n");
	return 0;
}
static void gk_session_checkout_progress_callback(const char *path, size_t cur, size_t tot, void *payload)
{
    /*
	progress_data *pd = (progress_data*)payload;
	pd->completed_steps = cur;
	pd->total_steps = tot;
	pd->path = path;
	print_progress(pd);*/
    printf("CHECKOUT PROGRESS\n");
}

void gk_session_clone(gk_session_t *session, git_credential *credential) {
    printf("DBG B1\n");
    if ((session == NULL)) {
        printf("DBG B2\n");
        return;
    }
    else if (session->repository == NULL) {
        printf("DBG B4\n");
        gk_session_set_last_result(session, gk_result(-1, "Cannot clone, session repository is NULL"));
        return;
    }

    printf("DBG B5\n");

    gk_authenticated_session_t authed_session;
    gk_authenticated_session_init(&authed_session, session, credential);

    git_repository *cloned_repo = NULL;
    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    int error;

    printf("DBG B6\n");
    /* Set up options */
    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_opts.progress_cb = gk_session_checkout_progress_callback;
    //checkout_opts.progress_payload = &pd;
    clone_opts.checkout_opts = checkout_opts;
    //clone_opts.fetch_opts.callbacks.sideband_progress = sideband_progress;
    clone_opts.fetch_opts.callbacks.transfer_progress = &gk_session_fetch_progress_callback;
    clone_opts.fetch_opts.callbacks.credentials = &gk_session_credential_callback;
    clone_opts.fetch_opts.callbacks.payload = &authed_session;

    printf("DBG B7: %s\n", session->repository->remote_url);
    printf("DBG B8: %s\n", session->repository->local_path);
    /* Do the clone */
    log_info(COMP_CLONE, "Cloning repo\n  URL:%s\n  local path:%s\n", session->repository->remote_url, session->repository->local_path);
    error = git_clone(&cloned_repo, session->repository->remote_url, session->repository->local_path, &clone_opts);

    if (error != 0) {
        const git_error *err = git_error_last();
        if (err) {
            gk_session_set_last_result(session, gk_result(err->klass, err->message));
        }
        else {
            gk_session_set_last_result(session, gk_result(error, ""));
        }
    }
    else {
        gk_session_set_last_result(session, gk_result_success());   
    }
    if (cloned_repo) {
        git_repository_free(cloned_repo);
    }
}
