
#ifndef __GITKEBAB_SESSION_PROGRESS_H__
#define __GITKEBAB_SESSION_PROGRESS_H__

#include "gk_types.h"

int gk_session_fetch_progress_callback(const void *stats_vptr, void *payload);
void gk_session_checkout_progress_callback(const char *path, size_t cur, size_t tot, void *payload);
int gk_session_progress_push_transfer_callback(unsigned int current, unsigned int total, size_t bytes, void *payload);
int gk_session_progress_push_update_reference_callback(const char *refname, const char *status, void *data);
int gk_session_transport_message_callback(const char *str, int len, void *payload);

gk_session_progress *gk_session_progress_init_fetch(size_t received_bytes, unsigned int total_objects, unsigned int total_deltas, unsigned int received_objects, unsigned int indexed_objects, unsigned int indexed_deltas);
gk_session_progress *gk_session_progress_init_checkout(const char *path, size_t cur, size_t tot);
gk_session_progress *gk_session_progress_init_push_transfer(unsigned int current, unsigned int total, size_t bytes);
void gk_session_progress_free(gk_session_progress *progress);
void gk_session_progress_update(gk_session *session, gk_session_progress *progress);

#endif // __GITKEBAB_SESSION_PROGRESS_H__
