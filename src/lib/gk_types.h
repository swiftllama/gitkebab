
#ifndef __GK_TYPES_H__
#define __GK_TYPES_H__

typedef struct {
    short int local_checkout_exists;
    short int has_conflicts;
    short int merge_in_progress;
    short int clone_in_progress;
    short int push_in_progress;
    short int pull_in_progress;
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
