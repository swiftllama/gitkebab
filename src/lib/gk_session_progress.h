
#ifndef __GITKEBAB_SESSION_PROGRESS_H__
#define __GITKEBAB_SESSION_PROGRESS_H__

#include "gk_types.h"

int gk_session_fetch_progress_callback(const void *stats_vptr, void *payload);
void gk_session_checkout_progress_callback(const char *path, size_t cur, size_t tot, void *payload);
int gk_session_progress_push_transfer_callback(unsigned int current, unsigned int total, size_t bytes, void *payload);

void gk_session_progress_init_fetch(gk_session_progress *progress, size_t received_bytes, unsigned int total_objects, unsigned int total_deltas, unsigned int received_objects, unsigned int indexed_objects, unsigned int indexed_deltas);
void gk_session_progress_init_checkout(gk_session_progress *progress, const char *path, size_t cur, size_t tot);


#endif // __GITKEBAB_SESSION_PROGRESS_H__
