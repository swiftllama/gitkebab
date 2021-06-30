
#ifndef __GITKEBAB_SESSION_PROGRESS_H__
#define __GITKEBAB_SESSION_PROGRESS_H__

enum gk_session_progress_event_type {
    GK_SESSION_PROGRESS_FETCH, GK_SESSION_PROGRESS_CHECKOUT
};
    
typedef struct {
    int network_percent;
    int index_percent;
    size_t received_bytes;
    int deltas_resolved_percent;
} gk_fetch_progress_t;
    
typedef struct {
    size_t completed_steps;
    size_t total_steps;
    int checkout_percent;
    const char* current_path;
} gk_checkout_progress_t;

typedef struct {
    gk_fetch_progress_t fetch;
    gk_checkout_progress_t checkout;
    int progress_event_type;
    int percent;
    char description[256];
} gk_session_progress_t;

typedef void gk_session_progress_callback_t(gk_session_progress_t *progress);

void gk_session_fetch_progress_callback(const void *stats_vptr, void *payload);
void gk_session_checkout_progress_callback(const char *path, size_t cur, size_t tot, void *payload);

void gk_session_progress_init_fetch(gk_session_progress_t *progress, size_t received_bytes, unsigned int total_objects, unsigned int total_deltas, unsigned int received_objects, unsigned int indexed_objects, unsigned int indexed_deltas);
void gk_session_progress_init_checkout(gk_session_progress_t *progress, const char *path, size_t cur, size_t tot);


#endif // __GITKEBAB_SESSION_PROGRESS_H__
