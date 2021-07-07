
#ifndef __GK_TYPES_H__
#define __GK_TYPES_H__

#include <stdio.h>

enum GK_RESULT_CODE {
    GK_SUCCESS, GK_FAILURE
};

typedef struct {
    int code;
    char *message;
} gk_result;


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

typedef struct {
    short int local_checkout_exists;
    short int has_conflicts;
    short int has_changes_to_merge;
    short int merge_in_progress;
    short int clone_in_progress;
    short int push_in_progress;
    short int fetch_in_progress;
} gk_session_state;
    
typedef struct {
    const char *local_path;
    const char *remote_url;
    const char *user;
} gk_repository;

typedef struct {
    size_t count_new;
    size_t count_modified;
    size_t count_deleted;
    size_t count_renamed;
    size_t count_typechange;
    size_t count_conflicted;

} gk_status_summary;

typedef struct {
    gk_session_progress_callback *progress_callback;
} gk_session_callbacks;

typedef struct {
    gk_repository repository;
    gk_result *last_result;
    gk_session_callbacks callbacks;
    gk_session_state state;
    gk_status_summary status_summary;

    void *lg2_status_list;
    void *lg2_repository;
} gk_session;

typedef struct {
    gk_session *session;
    gk_session_credential *credential;
} gk_authenticated_session;

#endif // __GK_TYPES_H__
