
#ifndef __GK_CONFLICTS_H__
#define __GK_CONFLICTS_H__

#include "gk_types.h"

gk_merge_conflict_entry *gk_merge_conflict_entry_new();
void gk_merge_conflict_entry_free(gk_merge_conflict_entry *entry);
const char *gk_merge_conflict_entry_type_string(gk_merge_conflict_entry_type entry_type);

int gk_conflicts_allocate(gk_repository *repository, size_t num_conflicts);
void gk_conflicts_free(gk_repository *repository);

gk_conflict_diff_summary *gk_conflict_diff_summary_new();
void gk_conflict_diff_summary_free(gk_conflict_diff_summary *summary);

gk_void_linked_node *gk_void_linked_node_new();
void gk_free_void_node_chain(gk_void_linked_node *chain, int free_data);

int gk_conflict_resolve_accept_existing(gk_session *session, const char *path, gk_conflict_resolution accept);
int gk_blob_contents(gk_session *session, void **blob_data, size_t *blob_data_length, const char *oid_id);
const char *gk_blob_new_char_contents(gk_session *session, const char *oid_id);
void gk_blob_free_char_contents(const char *contents);

int gk_blob_write_contents(gk_session *session, const char *oid_id, const char *path, int relativize_path);
int gk_conflict_resolve_accept_remote_delete(gk_session *session, const char *path);
int gk_conflict_resolve_accept_local_delete(gk_session *session, const char *path);

const char *gk_conflict_merged_buffer_with_conflict_markers(gk_session *session, const char *ancestor_oid_id, const char *ours_oid_id, const char *theirs_oid_id, const char *path);
void gk_conflict_merged_buffer_free(const char *buffer);


int gk_conflict_resolve_from_buffer(gk_session *session, const char *path, void *data, size_t data_length);
int gk_compare_blobs(gk_session *session, int *similarity, const char *blob1_oid_id, const char *blob2_oid_id);

#endif // __GK_CONFLICTS_H__
