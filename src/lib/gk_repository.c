
#include "stdio.h"
#include <string.h>

#include "git2.h"
#include "gk_repository.h"
#include "gk_results.h"
#include "gk_logging.h"
#include "gk_status.h"
#include "gk_conflicts.h"
#include "gk_lg2_private.h"
#include "gk_filesystem.h"
#include "gk_execution_context.h"
#include "gk_session.h"

static void gk_repository_spec_init(gk_repository_spec *repository_spec, const char *source_url, gk_repository_source_url_type source_url_type,  const char *local_path, const char *usr) {
    if (repository_spec == NULL) {
        return;
    }
    if (source_url == NULL) {
        log_warn(COMP_GENERAL, "Repository spec initialized with NULL source url, will use empty string instead");
    }
    if (local_path == NULL) {
        log_warn(COMP_GENERAL, "Repository spec initialized with NULL local_url, will use empty string instead");
    }
    if (usr == NULL) {
        log_warn(COMP_GENERAL, "Repository spec initialized with NULL user, will use empty string instead");
    }
    repository_spec->local_path = local_path != NULL ? strdup(local_path) : strdup("");
    repository_spec->source_url = source_url != NULL ? strdup(source_url) : strdup("");
    repository_spec->source_url_type = source_url_type;
    repository_spec->user = usr != NULL ? strdup(usr) : strdup("");
    repository_spec->main_branch_name = "main";
    repository_spec->remote_ref_name = "refs/remotes/origin/main";
    repository_spec->remote_name = "origin";
    repository_spec->push_refspec = "refs/heads/main:refs/heads/main";
}

static void gk_repository_spec_free_members(gk_repository_spec *repository_spec) {
    free((char *)repository_spec->local_path);
    repository_spec->local_path = NULL;
    free((char *)repository_spec->source_url);
    repository_spec->source_url = NULL;
    free((char *)repository_spec->user);
    repository_spec->user = NULL;
}

gk_repository *gk_repository_new() {
    // NOTE: new repository memory must be zerored out
    gk_repository *repository = (gk_repository *)calloc(1, sizeof(gk_repository));
    repository->state_counter = 0;
    repository->lg2_resources = (gk_lg2_resources *)calloc(1, sizeof(gk_lg2_resources));
    return repository;
}


void gk_repository_init(gk_repository *repository, const char *source_url, gk_repository_source_url_type source_url_type, const char *local_path, const char *user) {
    if (repository == NULL) {
        return;
    }
    gk_repository_spec_init(&repository->spec, source_url, source_url_type, local_path, user);

    repository->state = 0;
    gk_status_summary_reset(&repository->status_summary);
    
    gk_lg2_resources_init(repository);

    repository->conflict_summary.num_conflicts = 0;
    repository->conflict_summary.conflicts = NULL;
}

void gk_repository_update_remote(gk_repository *repository, const char* remote_url) {
    if (repository == NULL) return;
    free((void *)repository->spec.source_url);
    repository->spec.source_url = remote_url == NULL ? strdup("") : strdup(remote_url);
}

int gk_open_local_repository(gk_session *session) {
    const char *purpose = "open local repository";
    if (gk_session_context_push(session, purpose, &COMP_REPOSITORY, GK_REPOSITORY_VERIFY_NONE) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    if (gk_directory_exists(session->repository->spec.local_path) == 0) {
        return gk_session_failure_ex(session, purpose, GK_ERR_LOCAL_REPOSITORY_INEXISTENT_SOURCE_PATH, "local repository source path [%s] does not exist", session->repository->spec.local_path);
    }
    if (gk_lg2_repository_open(session) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }

    gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS);
    return gk_session_success(session, purpose);
}

int gk_create_local_repository(gk_session *session, const char *main_branch_name) {
    const char *purpose = "create local repository";
    if (gk_session_context_push(session, purpose, &COMP_REPOSITORY, GK_REPOSITORY_VERIFY_NONE) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    
    if (gk_lg2_repository_init_ext(session, main_branch_name) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }

    gk_repository_state_set(session->repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS);
    return gk_session_success(session, purpose);
}

void gk_repository_free(gk_repository *repository) {
    if (repository == NULL) {
        return;
    }

    gk_repository_spec_free_members(&repository->spec);
    gk_conflicts_free(repository);
    gk_lg2_free_all_but_repository(repository);
    gk_lg2_repository_free(repository);
    free(repository->lg2_resources);
    free(repository);
}

int gk_repository_state_enabled(gk_repository *repository, unsigned int states) {
    if (repository == NULL) {
        log_warn(COMP_REPOSITORY, "gk_repository_state_enabled called on NULL repository");
        return 0;
    }
    return (repository->state & states) == states;
}

int gk_repository_state_disabled(gk_repository *repository, unsigned int states) {
    if (repository == NULL) {
        log_warn(COMP_REPOSITORY, "gk_repository_state_disabled called on NULL repository");
        return 0;
    }
    return (repository->state & states) == 0;
}

void gk_repository_state_set(gk_repository *repository, int states_enable) {
    if (repository == NULL) {
        log_warn(COMP_REPOSITORY, "gk_repository_state_set called on NULL repository");
        return;
    }
    log_info(COMP_REPOSITORY, "enable state(s) [%d]", states_enable);
    repository->state |= states_enable;
    repository->state_counter += 1; //note: overflow ok
}

void gk_repository_state_unset(gk_repository *repository, int states_disable) {
    if (repository == NULL) {
        log_warn(COMP_REPOSITORY, "gk_repository_state_unset called on NULL repository");
        return;
    }
    log_info(COMP_REPOSITORY, "disable state(s) [%d]", states_disable);
    repository->state &= ~states_disable;
    repository->state_counter += 1; //note: overflow ok
}

void gk_repository_print_state(gk_repository *repository) {
    log_info(COMP_REPOSITORY, "Repository state:\n");
    log_info(COMP_REPOSITORY, "  GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS: : %d", gk_repository_state_enabled(repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS));
    log_info(COMP_REPOSITORY, "  GK_REPOSITORY_STATE_HAS_CONFLICTS: : %d", gk_repository_state_enabled(repository, GK_REPOSITORY_STATE_HAS_CONFLICTS));
    log_info(COMP_REPOSITORY, "  GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE: : %d", gk_repository_state_enabled(repository, GK_REPOSITORY_STATE_HAS_CHANGES_TO_MERGE));
    log_info(COMP_REPOSITORY, "  GK_REPOSITORY_STATE_CLONE_IN_PROGRESS: : %d", gk_repository_state_enabled(repository, GK_REPOSITORY_STATE_CLONE_IN_PROGRESS));
    log_info(COMP_REPOSITORY, "  GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING: : %d", gk_repository_state_enabled(repository, GK_REPOSITORY_STATE_MERGE_FINALIZATION_PENDING));
    log_info(COMP_REPOSITORY, "  GK_REPOSITORY_STATE_MERGE_PENDING_ON_DISK: : %d", gk_repository_state_enabled(repository, GK_REPOSITORY_STATE_MERGE_PENDING_ON_DISK));
    log_info(COMP_REPOSITORY, "  GK_REPOSITORY_STATE_PUSH_IN_PROGRESS: : %d", gk_repository_state_enabled(repository, GK_REPOSITORY_STATE_PUSH_IN_PROGRESS));
    log_info(COMP_REPOSITORY, "  GK_REPOSITORY_STATE_FETCH_IN_PROGRESS: : %d", gk_repository_state_enabled(repository, GK_REPOSITORY_STATE_FETCH_IN_PROGRESS));
    log_info(COMP_REPOSITORY, "  GK_REPOSITORY_STATE_MERGE_IN_PROGRESS: : %d\n", gk_repository_state_enabled(repository, GK_REPOSITORY_STATE_MERGE_IN_PROGRESS));
}

int gk_prepend_repository_path(gk_session *session, char *buffer, size_t buffer_length, const char *path) {
    const char *purpose = "prepend repository path";
    if (gk_session_context_push(session, purpose, &COMP_REPOSITORY, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    if (buffer == NULL) {
        return gk_session_failure_ex(session, purpose, GK_ERR, "Failed to prepend repository path to [%s], destination buffer is NULL", path);
    }

    if (gk_concatenate_paths(buffer, buffer_length, session->repository->spec.local_path, path) != 0) {
        return gk_session_failure_ex(session, purpose, GK_ERR, "Error prepending repository path [%s] to [%s]", session->repository->spec.local_path, path);
    }
    
    return gk_session_success(session, purpose);
}


