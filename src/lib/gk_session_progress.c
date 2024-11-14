
#include <string.h>

#include "git2.h"

#include "gk_repository.h"
#include "gk_session.h"
#include "gk_logging.h"

static int gk_session_check_progress_pointer(void *payload, const char *progress_type) {
    if (payload == NULL) {
        log_error(COMP_PROGRESS, "Session payload is NULL in %s progress callback", progress_type);
        return -1;
    }
    gk_session *session = (gk_session *)payload;
    if (session->repository == NULL) {
        log_error(COMP_PROGRESS, "Session contains NULL repository in %s progress callback", progress_type);
        return -1;
    }
    return 0;
}

static gk_session_progress *gk_session_progress_init(int progress_event_type) {
    gk_session_progress *progress = malloc(sizeof(gk_session_progress));
    memset(progress, 0, sizeof(gk_session_progress));

    progress->percent = 0;
    progress->description[0] = '\0';
    progress->progress_event_type = progress_event_type;

    if (progress_event_type == GK_SESSION_PROGRESS_CHECKOUT) {
        progress->checkout = malloc(sizeof(gk_checkout_progress));
        memset(progress->checkout, 0, sizeof(gk_checkout_progress));
    }
    else if (progress_event_type == GK_SESSION_PROGRESS_FETCH) {
        progress->fetch = malloc(sizeof(gk_fetch_progress));
        memset(progress->fetch, 0, sizeof(gk_fetch_progress));
    }
    else if (progress_event_type == GK_SESSION_PROGRESS_PUSH_TRANSFER) {
        progress->push_transfer = malloc(sizeof(gk_push_transfer_progress));
        memset(progress->push_transfer, 0, sizeof(gk_push_transfer_progress));
    }

    return progress;
}

gk_session_progress *gk_session_progress_init_push_transfer(unsigned int current, unsigned int total, size_t bytes) {
    gk_session_progress *progress = gk_session_progress_init(GK_SESSION_PROGRESS_PUSH_TRANSFER);
    progress->push_transfer->current = current;
    progress->push_transfer->total = total;
    progress->push_transfer->bytes = bytes;
    progress->push_transfer->percent = total == 0 ? 0 : (int)(100.0*(float)current / (float)total);

    progress->percent = progress->push_transfer->percent;
    snprintf(progress->description, 1024, "Uploading %d%% (%zuB)", progress->push_transfer->percent, progress->push_transfer->bytes);
    progress->description_ptr = progress->description;

    return progress;
}

gk_session_progress *gk_session_progress_init_fetch(size_t received_bytes, unsigned int total_objects, unsigned int total_deltas, unsigned int received_objects, unsigned int indexed_objects, unsigned int indexed_deltas) {
    gk_session_progress *progress = gk_session_progress_init(GK_SESSION_PROGRESS_FETCH);

    progress->fetch->received_bytes = received_bytes;
    progress->fetch->network_percent = total_objects == 0 ? 0 : (int)((100*received_objects) / total_objects);
    progress->fetch->index_percent = total_objects == 0 ? 0 : (int)((100*indexed_objects) / total_objects);
    progress->fetch->deltas_resolved_percent = total_deltas == 0 ? 0 : (int)(100*indexed_deltas / total_deltas);

    // Approximate progress->percent as a linear function of the three different percentages
    // this mostly works but is prone to sudden jumps
    if ((total_objects == 0) && (total_deltas == 0)) {
        snprintf(progress->description, 256, "Receiving objects");
        progress->percent = 0;
    }
    if (total_objects > 0) {
        if (received_objects < total_objects) {
            snprintf(progress->description, 256, "Receiving objects");
            progress->percent = (int)(0.33*(progress->fetch->network_percent + progress->fetch->index_percent));
        }
        else if (indexed_objects < total_objects) {
            snprintf(progress->description, 256, "Indexing objects");
            progress->percent = (int)(0.33*(progress->fetch->network_percent + progress->fetch->index_percent));
        }
    }
    if (progress->description[0] == '\0') {
        snprintf(progress->description, 1024, "Resolving deltas");
        progress->percent = 67 + (int)(0.33*(progress->fetch->deltas_resolved_percent));
    }
    progress->description_ptr = progress->description;

    return progress;
}

gk_session_progress *gk_session_progress_init_checkout(const char *path, size_t current_steps, size_t total_steps) {

    gk_session_progress *progress = gk_session_progress_init(GK_SESSION_PROGRESS_CHECKOUT);

    progress->checkout->completed_steps = current_steps;
    progress->checkout->total_steps = total_steps;
    progress->checkout->checkout_percent = total_steps == 0 ? 0 : (int)(100*(float)current_steps/(float)total_steps);
    progress->checkout->current_path = path == NULL ? "" : path;

    progress->percent = progress->checkout->checkout_percent;
    progress->percent = total_steps == 0 ? 0 : (int)(100*(float)current_steps/(float)total_steps);
    snprintf(progress->description, 1024, "%s", path == NULL ? "" : path);
    progress->description_ptr = progress->description;

    return progress;
}

void gk_session_progress_free(gk_session_progress *progress) {
    if (progress == NULL) {
        return;
    }

    if (progress->checkout != NULL) {
        free(progress->checkout);
        progress->checkout = NULL;
    }
    if (progress->fetch != NULL) {
        free(progress->fetch);
        progress->fetch = NULL;
    }
    if (progress->push_transfer != NULL) {
        free(progress->push_transfer);
        progress->push_transfer = NULL;
    }
}

int gk_session_fetch_progress_callback(const void *stats_vptr, void *payload) {
    git_indexer_progress *stats = (git_indexer_progress *)stats_vptr;
    if (gk_session_check_progress_pointer(payload, "fetch") != 0) {
        return 0;
    }
    if (stats == NULL) {
        log_error(COMP_PROGRESS, "stats are NULL in fetch progress callback");
        return 0;
    }
    
    gk_session *session = (gk_session *)payload;
    gk_session_progress *progress =  gk_session_progress_init_fetch(stats->received_bytes, stats->total_objects, stats->total_deltas, stats->received_objects, stats->indexed_objects, stats->indexed_deltas);
    if (session->callbacks.state_changed_callback != NULL) {
        session->callbacks.state_changed_callback(session->id_ptr, session->repository, progress);
    }
    gk_session_progress_update(session, progress);    
    return 0;
}


void gk_session_checkout_progress_callback(const char *path, size_t current_steps, size_t total_steps, void *payload) {
    if (gk_session_check_progress_pointer(payload, "checkout") != 0) {
        return;
    }

    gk_session *session = (gk_session *)payload;
    gk_session_progress *progress = gk_session_progress_init_checkout(path, current_steps, total_steps);
    if (session->callbacks.state_changed_callback != NULL) {
        session->callbacks.state_changed_callback(session->id_ptr, session->repository, progress);
    }
    gk_session_progress_update(session, progress);
    gk_session_progress_free(progress);
}

int gk_session_progress_push_transfer_callback(unsigned int current, unsigned int total, size_t bytes, void *payload) {
    if (gk_session_check_progress_pointer(payload, "push transfer") != 0) {
        return 0;
    }

    gk_session *session = (gk_session *)payload;
    gk_session_progress *progress = gk_session_progress_init_push_transfer(current, total, bytes);
    if (session->callbacks.state_changed_callback != NULL) {
        session->callbacks.state_changed_callback(session->id_ptr, session->repository, progress);
    }
    gk_session_progress_update(session, progress);
    gk_session_progress_free(progress);
    return 0;
}

void gk_session_progress_update(gk_session *session, gk_session_progress *progress) {
    if (gk_session_state_lock(session) == GK_SUCCESS) {
        gk_session_progress_free(session->progress);
        session->progress = progress;
        session->repository->state_counter += 1;
        gk_session_state_unlock(session);
    }
}

int gk_session_progress_push_update_reference_callback(const char *refname, const char *status, void *payload) {
    (void) refname; // unused
    (void) payload; //unused
    if (status != NULL) {
        const char *purpose = "push references";
        if (payload != NULL) {
            gk_session *session = (gk_session *)payload;
            if (gk_session_last_result_code(session) == GK_SUCCESS) {
                if (gk_session_context_push(session, purpose, &COMP_AUTH, GK_REPOSITORY_VERIFY_INITIALIZED) != GK_SUCCESS) {
                    return -1;
                }
                gk_session_failure_ex(session, purpose, GK_ERR, status);
                return -1;
            }
            else {
                log_error(COMP_PROGRESS, "session payload is unexpectedly NULL during transport message callback");
            }
        }
        log_error(COMP_PROGRESS, "error while pushing references: %s", status);
        return -1;

    }
    return 0;
}

int gk_session_transport_message_callback(const char *str, int len, void *payload) {
    (void) payload; // unused
    if (str != NULL) {
        const int prefix_size = 16;
        if ((len >= prefix_size) && (strncmp(str, "JEMDRIVE-ERROR: ", prefix_size) == 0)){
            char message[1024];
            snprintf(message, len - prefix_size > 1024 ? 1024 : len - prefix_size, "%s", str + prefix_size);
            const char *purpose = "process transport messages";
            if (payload != NULL) {
                gk_session *session = (gk_session *)payload;
                if (gk_session_context_push(session, purpose, &COMP_AUTH, GK_REPOSITORY_VERIFY_INITIALIZED) != GK_SUCCESS) {
                    return -1;
                }
                gk_session_failure_ex(session, purpose, GK_ERR, message);
                return -1;
            }
            else {
                log_error(COMP_PROGRESS, "error received from server: %s", message);
                log_error(COMP_PROGRESS, "session payload is unexpectedly NULL during transport message callback");
                return -1;
            }   
        }
        else {
            char message[1024];
            snprintf(message, len > 1024 ? 1024 : len, "%s", str);
            log_info(COMP_PROGRESS, "remote: %s", message);
        }
    }
    return 0;
}
