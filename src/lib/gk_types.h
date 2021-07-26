
#ifndef __GK_TYPES_H__
#define __GK_TYPES_H__

#include <stdio.h>
#include "rxi_log.h"


typedef struct gk_repository gk_repository;
typedef struct gk_lg2_resources gk_lg2_resources;
typedef struct gk_execution_context gk_execution_context;

enum GK_RESULT_CODE {
    GK_SUCCESS, GK_FAILURE, GK_ERR
};

typedef enum {
    GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_EDIT,
    GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_CREATE,
    GK_MERGE_CONFLICT_LOCAL_EDIT_REMOTE_DELETE,
    GK_MERGE_CONFLICT_LOCAL_DELETE_REMOTE_EDIT
} gk_merge_conflict_entry_type;

typedef enum {
    GK_REPOSITORY_VERIFY_DEFAULT = (1 << 0),
    GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT = (1 << 1),
    GK_REPOSITORY_VERIFY_STATUS_LIST = (1 << 2),
    GK_REPOSITORY_VERIFY_MERGE_IN_PROGRESS = (1 << 3)
} gk_repository_verify_condition;

typedef struct {
    int code;
    char *message;
} gk_result;

struct gk_execution_context {
    const char *purpose;
    gk_result *result;
    gk_execution_context *child_context;
    log_Component *log_component;
};


enum gk_session_progress_event_type {
    GK_SESSION_PROGRESS_FETCH, GK_SESSION_PROGRESS_CHECKOUT, GK_SESSION_PROGRESS_PUSH_TRANSFER
};
    
typedef struct {
    int network_percent;
    int index_percent;
    size_t received_bytes;
    int deltas_resolved_percent;
} gk_fetch_progress;
    
typedef struct {
    size_t completed_steps;
    size_t total_steps;
    int checkout_percent;
    const char* current_path;
} gk_checkout_progress;

typedef struct {
    gk_merge_conflict_entry_type conflict_type;
    char *path;
    char ancestor_oid_id[41];
    char ours_oid_id[41];
    char theirs_oid_id[41];
} gk_merge_conflict_entry;

typedef struct {
    size_t num_conflicts;
    gk_merge_conflict_entry **conflicts;
    char repository_head_oid_id[41];
    char fetch_head_oid_id[41];
} gk_merge_conflict_summary;

typedef struct {
    const char *ancestor_to_ours_diff;
    const char *ancestor_to_theirs_diff;
} gk_conflict_diff_summary;

typedef struct {
    unsigned int current;
    unsigned int total;
    size_t bytes;
    int percent;
} gk_push_transfer_progress;

typedef struct {
    gk_fetch_progress fetch;
    gk_checkout_progress checkout;
    gk_push_transfer_progress push_transfer;
    int progress_event_type;
    int percent;
    char description[256];
} gk_session_progress;

typedef void gk_session_progress_callback(gk_session_progress *progress);

typedef enum {
    CREDENTIAL_SSH_KEY_MEMORY,
    CREDENTIAL_SSH_KEY_FILE,
    CREDENTIAL_USERNAME_PASSWORD
} gk_session_credential_type;
    
typedef struct {
    gk_session_credential_type credential_type;
    const char *ssh_private_key_bytes;
    const char *ssh_public_key_bytes;
    const char *ssh_private_key_path;
    const char *ssh_public_key_path;
    const char *ssh_private_key_passphrase;
    const char *username;
    const char *password;
} gk_session_credential;

typedef enum {
    GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS      = (1 << 0),
    GK_REPOSITORY_STATE_HAS_CONFLICTS              = (1 << 1),
    GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE       = (1 << 2),
    GK_REPOSITORY_STATE_CLONE_IN_PROGRESS          = (1 << 3),
    GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING = (1 << 4),
    GK_REPOSITORY_STATE_MERGE_PENDING_ON_DISK      = (1 << 5),
    GK_REPOSITORY_STATE_PUSH_IN_PROGRESS           = (1 << 6),
    GK_REPOSITORY_STATE_FETCH_IN_PROGRESS          = (1 << 7),
    GK_REPOSITORY_STATE_MERGE_IN_PROGRESS          = (1 << 8),
} gk_repository_state;
    
typedef struct {
    const char *local_path;
    const char *remote_url;
    const char *user;
    const char *main_branch_name;
    const char *remote_ref_name;
    const char *remote_name;
    const char *push_refspec;
} gk_repository_spec;

typedef struct {
    size_t count_new;
    size_t count_modified;
    size_t count_deleted;
    size_t count_renamed;
    size_t count_typechange;
    size_t count_conflicted;
} gk_status_summary;

typedef void gk_repository_state_changed_callback(gk_repository *repository);

typedef struct {
    gk_session_progress_callback *progress_callback;
    gk_repository_state_changed_callback *state_changed_callback;
} gk_session_callbacks;

struct gk_repository {
    gk_repository_spec repository_spec;
    gk_session_callbacks callbacks;
    gk_repository_state state;
    gk_status_summary status_summary;
    gk_merge_conflict_summary conflict_summary;
    
    gk_lg2_resources *lg2_resources;
};

typedef struct {
    gk_execution_context *context;
    gk_repository *repository;
    gk_session_credential credential;
} gk_session;


typedef struct gk_void_linked_node gk_void_linked_node;

struct gk_void_linked_node {
    void *data;
    gk_void_linked_node *next;
};

typedef enum {
    GK_CONFLICT_RESOLUTION_OURS,
    GK_CONFLICT_RESOLUTION_THEIRS,
    GK_CONFLICT_RESOLUTION_ANCESTOR,
} gk_conflict_resolution;

#endif // __GK_TYPES_H__
