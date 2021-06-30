
#include "gk_sesssion.h"

void gk_session_fetch_progress_callback(const git_indexer_progress *stats, void *payload) {
    if (payload == NULL) {
        log_ERROR(COMP_PROGRESS, "Authed-session payload is NULL in fetch progress callback");
        return;
    }
    gk_authenticated_session_t *authed_session = (gk_authenticated_session_t *)payload;
    if (authed_session->session == NULL) {
        log_error(COMP_PROGRESS, "Authed-session contains NULL in fetch progress callback");
        return;
    }
    if (stats == NULL) {
        log_error(COMP_PROGRESS, "stats are NULL in fetch progress callback");
        return;
    }

    gk_session_progress_t session_progress;
    // TODO: interpret payload as authed session, update progress from stats in session
    /*
	progress_data *pd = (progress_data*)payload;
	pd->fetch_progress = *stats;
	print_progress(pd);*/
    printf("FETCH PROGRESS\n");
	return 0;
}

void gk_session_checkout_progress_callback(const char *path, size_t cur, size_t tot, void *payload) {
    // TODO: interpret payload as authed session, update progress from stats in session
    /*
	progress_data *pd = (progress_data*)payload;
	pd->completed_steps = cur;
	pd->total_steps = tot;
	pd->path = path;
	print_progress(pd);*/
    printf("CHECKOUT PROGRESS\n");
}


void gk_session_progress_init_fetch(gk_session_progress_t *progress) {
    if (progress == NULL) {
        log_error(COMP_PROGRESS, "Cannot initialize NULL session progress as fetch progress");
        return;
    }

    // TODO: initialize session progress
}

void gk_session_progress_init_checkout(gk_session_progress_t *progress) {
    if (progress == NULL) {
        log_error(COMP_PROGRESS, "Cannot initialize NULL session progress as checkout progress");
        return;
    }

    // TODO: initialize session progress
}
