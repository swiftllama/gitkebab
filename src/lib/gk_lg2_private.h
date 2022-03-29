
#ifndef _GK_LG2_PRIVATE_H__
#define _GK_LG2_PRIVATE_H__

#include "git2.h"

struct gk_lg2_resources {
    git_reference *repository_head_ref;
    git_object *repository_head_object;
    git_commit *repository_head_commit;
    const git_oid *repository_head_oid;
    char repository_head_oid_id[41];
    
    git_reference *fetch_head_ref;
    git_object *fetch_head_object;
    git_commit *fetch_head_commit;
    git_annotated_commit *annotated_fetch_head_commit;
    const git_oid *fetch_head_oid;
    char fetch_head_oid_id[41];

    git_oid tree_oid;
    char tree_oid_id[41];
    
    git_reflog *reflog;
    git_index *index;
    git_index *merge_index;
    git_tree *tree;

    git_signature *signature;

    git_commit **merge_parents;

    git_status_list *status_list;
    git_repository *repository;    
};

typedef struct {
    const git_index_entry *ancestor;
    const git_index_entry *ours;
    const git_index_entry *theirs;
    
    git_blob *ancestor_blob;
    git_blob *ours_blob;
    git_blob *theirs_blob;

    char ancestor_oid_id[41];
    char ours_oid_id[41];
    char theirs_oid_id[41];
}  gk_lg2_conflict_entry;

void gk_lg2_resources_init(gk_repository *repository);
void gk_lg2_free_all_but_repository(gk_repository *repository);
int gk_lg2_load_references(gk_session *session);
void gk_lg2_free_references(gk_repository *repository);
int gk_lg2_index_load(gk_session *session);
void gk_lg2_index_free(gk_repository *repository);
void gk_lg2_merge_index_free(gk_repository *repository);
int gk_lg2_promote_merge_index(gk_session *session);
int gk_lg2_repository_open(gk_session *session);
int gk_lg2_repository_init_ext(gk_session *session, const char *main_branch_name);
void gk_lg2_repository_free(gk_repository *repository);
int gk_lg2_reflog_read(gk_session *session, const char *ref_name);
void gk_lg2_reflog_free(gk_repository *gk_repository);
int gk_lg2_parents_lookup(gk_session *session);
void gk_lg2_parents_free(gk_repository *repository);
int gk_lg2_index_write_tree(gk_session *session, git_index *target_index);
void gk_lg2_tree_free(gk_repository *repository);
int gk_lg2_signature_create(gk_session *session);
void gk_lg2_signature_free(gk_repository *repository);
void gk_lg2_status_list_free(gk_repository *repository);
int gk_lg2_status_list_load(gk_session *session);
int gk_lg2_checkout_tree(gk_session *session, git_checkout_options *checkout_options);
int gk_lg2_iterate_conflicts(gk_session *session);
void gk_lg2_conflict_entry_init(gk_lg2_conflict_entry *entry);
void gk_lg2_conflict_entry_free_members(gk_lg2_conflict_entry *entry);
gk_conflict_diff_summary *gk_lg2_conflict_diff_summary(gk_session *session, gk_merge_conflict_entry *entry);
int gk_lg2_index_conflict_get(gk_session *session, const git_index_entry **ancestor_entry, const git_index_entry **ours_entry, const git_index_entry **theirs_entry, git_index *index, const char *path);
int gk_lg2_index_add(gk_session *session, const git_index_entry *entry, const char *path);
int gk_lg2_oid_from_id(gk_session *session, git_oid *oid, const char *oid_id);
int gk_lg2_blob_lookup(gk_session *session, git_blob **blob, const git_oid *oid);
#endif // _GK_LG2_PRIVATE_H__
