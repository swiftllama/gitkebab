
#include "git2.h"

#include "gk_session.h"
#include "gk_logging.h"

static int gk_session_check_progress_pointer(void *payload, const char *progress_type) {
    if (payload == NULL) {
        log_error(COMP_PROGRESS, "Authed-session payload is NULL in %s progress callback", progress_type);
        return -1;
    }
    gk_authenticated_session_t *authed_session = (gk_authenticated_session_t *)payload;
    if (authed_session->session == NULL) {
        log_error(COMP_PROGRESS, "Authed-session contains NULL in %s progress callback", progress_type);
        return -1;
    }
    if (authed_session->session->callbacks.progress_callback == NULL) {
        log_info(COMP_PROGRESS, "Authed-session contains session with NULL progress callback, no callback will be invoked");
        return -1;
    }
    return 0;
}

static void gk_session_progress_init(gk_session_progress_t *progress) {
    progress->percent = 0;
    progress->description[0] = '\0';

    progress->fetch.network_percent = 0;
    progress->fetch.index_percent = 0;
    progress->fetch.received_bytes = 0;
    progress->fetch.deltas_resolved_percent = 0;

    progress->checkout.completed_steps = 0;
    progress->checkout.total_steps = 0;
    progress->checkout.checkout_percent = 0;
    progress->checkout.current_path = "";
}

void gk_session_progress_init_fetch(gk_session_progress_t *progress, size_t received_bytes, unsigned int total_objects, unsigned int total_deltas, unsigned int received_objects, unsigned int indexed_objects, unsigned int indexed_deltas) {    
    if (progress == NULL) {
        log_error(COMP_PROGRESS, "Cannot initialize NULL session progress as fetch progress");
        return;
    }

    gk_session_progress_init(progress);
    progress->progress_event_type = GK_SESSION_PROGRESS_FETCH;

    progress->fetch.received_bytes = received_bytes;
    progress->fetch.network_percent = total_objects == 0 ? 0 : (int)((100*received_objects) / total_objects);
    progress->fetch.index_percent = total_objects == 0 ? 0 : (int)((100*indexed_objects) / total_objects);
    progress->fetch.deltas_resolved_percent = total_deltas == 0 ? 0 : (int)(100*indexed_deltas / total_deltas);

    // Approximate progress->percent as a linear function of the three different percentages
    // this mostly works but is prone to sudden jumps
    if ((total_objects == 0) && (total_deltas == 0)) {
        snprintf(progress->description, 256, "Receiving objects");
        progress->percent = 0;
    }
    if (total_objects > 0) {
        if (received_objects < total_objects) {
            snprintf(progress->description, 256, "Receiving objects");
            progress->percent = (int)(0.33*(progress->fetch.network_percent + progress->fetch.index_percent));
        }
        else if (indexed_objects < total_objects) {
            snprintf(progress->description, 256, "Indexing objects");
            progress->percent = (int)(0.33*(progress->fetch.network_percent + progress->fetch.index_percent));
        }
    }
    if (progress->description[0] == '\0') {
        snprintf(progress->description, 256, "Resolving deltas");
        progress->percent = 67 + (int)(0.33*(progress->fetch.deltas_resolved_percent));
    }
}

void gk_session_progress_init_checkout(gk_session_progress_t *progress, const char *path, size_t current_steps, size_t total_steps) {
    if (progress == NULL) {
        log_error(COMP_PROGRESS, "Cannot initialize NULL session progress as checkout progress");
        return;
    }

    gk_session_progress_init(progress);
    progress->progress_event_type = GK_SESSION_PROGRESS_CHECKOUT;

    progress->checkout.completed_steps = current_steps;
    progress->checkout.total_steps = total_steps;
    progress->checkout.checkout_percent = total_steps == 0 ? 0 : (int)(100*(float)current_steps/(float)total_steps);
    progress->checkout.current_path = path == NULL ? "" : path;

    progress->percent = progress->checkout.checkout_percent;
    snprintf(progress->description, 256, "%s", path == NULL ? "" : path);
}

void gk_session_fetch_progress_callback(const void *stats_vptr, void *payload) {
    git_indexer_progress *stats = (git_indexer_progress *)stats_vptr;
    if (gk_session_check_progress_pointer(payload, "fetch") != 0) {
        return;
    }
    if (stats == NULL) {
        log_error(COMP_PROGRESS, "stats are NULL in fetch progress callback");
        return;
    }
    
    gk_authenticated_session_t *authed_session = (gk_authenticated_session_t *)payload;
    gk_session_progress_t progress;
    gk_session_progress_init_fetch(&progress, stats->received_bytes, stats->total_objects, stats->total_deltas, stats->received_objects, stats->indexed_objects, stats->indexed_deltas);
    authed_session->session->callbacks.progress_callback(&progress);
}


void gk_session_checkout_progress_callback(const char *path, size_t current_steps, size_t total_steps, void *payload) {
    if (gk_session_check_progress_pointer(payload, "checkout") != 0) {
        return;
    }

    gk_authenticated_session_t *authed_session = (gk_authenticated_session_t *)payload;
    gk_session_progress_t progress;
    gk_session_progress_init_checkout(&progress, path, current_steps, total_steps); 
    authed_session->session->callbacks.progress_callback(&progress);
}



