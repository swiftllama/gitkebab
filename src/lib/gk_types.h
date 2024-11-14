
#ifndef __GK_TYPES_H__
#define __GK_TYPES_H__

#include <stdio.h>
#include "rxi_log.h"


typedef struct gk_repository gk_repository;
typedef struct gk_lg2_resources gk_lg2_resources;
typedef struct gk_execution_context gk_execution_context;

enum GK_RESULT_CODE {
    GK_SUCCESS, GK_FAILURE, GK_ERR,
    // Local repository
    GK_ERR_LOCAL_REPOSITORY_INEXISTENT_SOURCE_PATH,
    // Clone
    GK_ERR_CLONE_INEXISTENT_SOURCE_PATH,
    GK_ERR_CLONE_INVALID_DESTINATION_PATH,
    GK_ERR_CLONE_DESTINATION_PATH_NONEMPTY,

    GK_ERR_REPOSITORY_NOT_INITIALIZED,
    GK_ERR_REPOSITORY_LOCAL_PATH_CONFLICT,
    GK_ERR_REPOSITORY_NO_LOCAL_CHECKOUT,
    GK_ERR_REPOSITORY_NOT_LOADED,
    GK_ERR_NOT_FOUND,

    GK_ERR_MERGE_HAS_CONFLICTS
};

typedef enum gk_merge_conflict_entry_type {
    GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_EDIT,
    GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_CREATE,
    GK_MERGE_CONFLICT_LOCAL_EDIT_REMOTE_DELETE,
    GK_MERGE_CONFLICT_LOCAL_DELETE_REMOTE_EDIT
} gk_merge_conflict_entry_type;

typedef enum gk_repository_verify_condition {
    GK_REPOSITORY_VERIFY_NONE               = (1 << 0),
    GK_REPOSITORY_VERIFY_INITIALIZED        = (1 << 1),
    GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT     = (1 << 2),
    GK_REPOSITORY_VERIFY_STATUS_LIST        = (1 << 3),
    GK_REPOSITORY_VERIFY_MERGE_IN_PROGRESS  = (1 << 4),
    GK_REPOSITORY_VERIFY_INDEX_LOADED       = (1 << 5),
    GK_REPOSITORY_VERIFY_MERGE_INDEX_LOADED = (1 << 6),
    GK_REPOSITORY_VERIFY_STATE_LOCK         = (1 << 7),
    GK_REPOSITORY_VERIFY_REPOSITORY_LOADED  = (1 << 8),
    GK_REPOSITORY_VERIFY_DEFAULT = GK_REPOSITORY_VERIFY_INITIALIZED | GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT | GK_REPOSITORY_VERIFY_INDEX_LOADED | GK_REPOSITORY_VERIFY_STATE_LOCK | GK_REPOSITORY_VERIFY_REPOSITORY_LOADED
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
    
typedef struct gk_fetch_progress {
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

typedef struct gk_conflict_diff_summary {
    const char *ancestor_to_ours_diff;
    const char *ancestor_to_theirs_diff;
} gk_conflict_diff_summary;

typedef struct gk_push_transfer_progress {
    unsigned int current;
    unsigned int total;
    size_t bytes;
    int percent;
} gk_push_transfer_progress;

typedef struct gk_session_progress {
    gk_fetch_progress *fetch;
    gk_checkout_progress *checkout;
    gk_push_transfer_progress *push_transfer;
    int progress_event_type;
    int percent;
    char description[1024];
    char *description_ptr; // Note: fixed-size char arrays are difficult to access in dart
    const char *session_id;
} gk_session_progress;

typedef enum gk_session_credential_type {
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

typedef enum gk_repository_state {
    GK_REPOSITORY_STATE_DEFAULT                     = 0,
    GK_REPOSITORY_STATE_INITIALIZED                 = (1 << 0),  // 1
    GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS       = (1 << 1),  // 2
    GK_REPOSITORY_STATE_HAS_CONFLICTS               = (1 << 2),  // 4
    GK_REPOSITORY_STATE_HAS_CHANGES_TO_COMMIT       = (1 << 3),  // 8
    GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE        = (1 << 4),  // 16
    GK_REPOSITORY_STATE_CLONE_IN_PROGRESS           = (1 << 5),  // 32
    GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING  = (1 << 6),  // 64
    GK_REPOSITORY_STATE_MERGE_PENDING_ON_DISK       = (1 << 7),  // 128
    GK_REPOSITORY_STATE_PUSH_IN_PROGRESS            = (1 << 8),  // 256
    GK_REPOSITORY_STATE_FETCH_IN_PROGRESS           = (1 << 9),  // 512
    GK_REPOSITORY_STATE_MERGE_IN_PROGRESS           = (1 << 10), // 1024
    GK_REPOSITORY_STATE_SYNC_IN_PROGRESS            = (1 << 11), // 2048
    GK_REPOSITORY_STATE_BACKGROUND_SYNC_IN_PROGRESS = (1 << 12), // 4096
} gk_repository_state;

#define GK_REPOSITORY_STATE_MAX_EXP 13

typedef enum gk_repository_source_url_type {
    GK_REPOSITORY_SOURCE_URL_SSH,
    GK_REPOSITORY_SOURCE_URL_HTTP,
    GK_REPOSITORY_SOURCE_URL_FILESYSTEM
} gk_repository_source_url_type;
    
typedef struct {
    const char *local_path;
    const char *source_url;
    gk_repository_source_url_type source_url_type;
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

typedef void gk_repository_state_changed_callback(const char *session_id, gk_repository *repository, gk_session_progress *progress);
typedef void gk_repository_did_query_merge_conflict_summary(const char *session_id, gk_repository *repository);

typedef struct {
    gk_repository_did_query_merge_conflict_summary *merge_conflict_query_callback;
    gk_repository_state_changed_callback *state_changed_callback;
} gk_session_callbacks;

struct gk_repository {
    gk_repository_spec spec;
    gk_repository_state state;
    int state_counter;
    gk_status_summary status_summary;
    gk_merge_conflict_summary conflict_summary;
    
    gk_lg2_resources *lg2_resources;
};

typedef struct {
    char id[16];
    char *id_ptr; // dart string access
    gk_execution_context *context;
    gk_repository *repository;
    gk_session_credential credential;
    gk_session_progress *progress;
    void *state_lock; // this is pthread_mutex_t, stored as void for dart compatibility
    gk_session_callbacks callbacks;
    gk_result *internal_last_result;
} gk_session;


typedef struct gk_void_linked_node gk_void_linked_node;

struct gk_void_linked_node {
    void *data;
    gk_void_linked_node *next;
};

typedef enum gk_conflict_resolution {
    GK_CONFLICT_RESOLUTION_OURS,
    GK_CONFLICT_RESOLUTION_THEIRS,
    GK_CONFLICT_RESOLUTION_ANCESTOR,
} gk_conflict_resolution;

typedef struct gk_remote {
    const char* name;
    const char* url;
} gk_remote;

typedef struct gk_remote_list {
    size_t count;
    gk_remote **remotes;
} gk_remote_list;


#endif // __GK_TYPES_H__
