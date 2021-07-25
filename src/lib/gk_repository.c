
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

static void gk_repository_spec_init(gk_repository_spec *repository_spec, const char *remote_url, const char *local_path, const char *usr) {
    if (repository_spec == NULL) {
        return;
    }
    if (remote_url == NULL) {
        log_warn(COMP_SESSION, "Repository spec initialized with NULL remote_url, will use empty string instead");
    }
    if (local_path == NULL) {
        log_warn(COMP_SESSION, "Repository spec initialized with NULL local_url, will use empty string instead");
    }
    if (usr == NULL) {
        log_warn(COMP_SESSION, "Repository spec initialized with NULL user, will use empty string instead");
    }
    repository_spec->local_path = local_path != NULL ? strdup(local_path) : strdup("");
    repository_spec->remote_url = remote_url != NULL ? strdup(remote_url) : strdup("");
    repository_spec->user = usr != NULL ? strdup(usr) : strdup("");
    repository_spec->main_branch_name = "master";
    repository_spec->remote_ref_name = "refs/remotes/origin/master";
    repository_spec->remote_name = "origin";
    repository_spec->push_refspec = "refs/heads/master:refs/heads/master";
}

static void gk_repository_spec_free_members(gk_repository_spec *repository_spec) {
    free((char *)repository_spec->local_path);
    repository_spec->local_path = NULL;
    free((char *)repository_spec->remote_url);
    repository_spec->remote_url = NULL;
    free((char *)repository_spec->user);
    repository_spec->user = NULL;
}

gk_repository *gk_repository_new() {
    gk_repository *repository = (gk_repository *)malloc(sizeof(gk_repository));
    repository->lg2_resources = (gk_lg2_resources *)malloc(sizeof(gk_lg2_resources));
    return repository;
}


void gk_repository_init(gk_repository *repository, const char *remote_url, const char *local_path, const char *user, gk_repository_progress_callback *progress_callback, gk_repository_state_changed_callback *state_changed_callback) {
    if (repository == NULL) {
        return;
    }
    gk_repository_spec_init(&repository->repository_spec, remote_url, local_path, user);
    repository->root_context = gk_execution_context_new("root context", &COMP_REPOSITORY);
    repository->last_result = gk_result_success();
    repository->callbacks.progress_callback = progress_callback;
    repository->callbacks.state_changed_callback = state_changed_callback;

    repository->state = 0;
    gk_status_summary_reset(&repository->status_summary);
    
    gk_lg2_resources_init(repository);

    repository->conflict_summary.num_conflicts = 0;
    repository->conflict_summary.conflicts = NULL;
}

int gk_open_local_repository(gk_session *session) {
    if (repository == NULL) {
        log_error(COMP_REPOSITORY, "gk_repository_open_local_repository(repository) called on NULL repository");
        return GK_FAILURE;
    }

    if (gk_lg2_repository_open(session, "open local repository") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_repository_state_set(repository, GK_REPOSITORY_STATE_LOCAL_CHECKOUT_EXISTS);

    if (git_repository_is_bare(repository->lg2_resources->repository) == 1) {
        gk_repository_state_unset(repository, GK_REPOSITORY_STATE_HAS_CONFLICTS);
        gk_repository_state_unset(repository, GK_REPOSITORY_STATE_MERGE_PENDING_ON_DISK);
    }
    else {
        int rc = gk_repository_status_summary_query(repository);
        gk_repository_status_summary_close(repository);
        if (rc != GK_SUCCESS) {
            return GK_FAILURE;
        }
    }

    return gk_repository_success(repository);
}

void gk_repository_set_last_result(gk_repository *repository, gk_result *last_result) {
    gk_result_free(repository->last_result);
    repository->last_result = last_result;
    if (last_result == NULL) {
        log_error(COMP_REPOSITORY, "Set a NULL last error on a repository, this could be the result of a memory allocation failure");
    }
}

void gk_repository_set_last_result_v(gk_repository *repository, int code, const char *message, ...) {
    va_list(args);
    va_start(args, message);
    gk_result *result = gk_result_v(code, message, args);
    va_end(args);
    gk_repository_set_last_result(repository, result);
}

void gk_repository_set_last_result_vargs(gk_repository *repository, int code, const char *message, va_list args) {
    gk_result *result = gk_result_vargs(code, message, args);
    gk_repository_set_last_result(repository, result);
}

int gk_repository_failure(gk_repository *repository, log_Component *component, int code, const char *message, ...) {
    va_list args;
    va_start(args, message);
    gk_repository_set_last_result_vargs(repository, code, message, args);
    va_end(args);
    log_log(LOG_ERROR, __FILE__, __LINE__, component, gk_result_message(repository->last_result));
    return GK_FAILURE;
}

int gk_repository_success(gk_repository *repository) {
    gk_repository_set_last_result(repository, gk_result_success());
    return GK_SUCCESS;
}

void gk_repository_free(gk_repository *repository) {
    if (repository == NULL) {
        return;
    }

    gk_result_free(repository->last_result);
    repository->last_result = NULL;
    gk_execution_context_free(repository->root_context);
    repository->root_context = NULL;
    gk_repository_spec_free_members(&repository->repository_spec);
    gk_conflicts_free(repository);
    gk_lg2_free_all_but_repository(repository);
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
    repository->state |= states_enable;
}

void gk_repository_state_unset(gk_repository *repository, int states_disable) {
    if (repository == NULL) {
        log_warn(COMP_REPOSITORY, "gk_repository_state_unset called on NULL repository");
        return;
    }
    repository->state &= ~states_disable;
}

void gk_repository_state_trigger_callback(gk_repository *repository) {
    if (repository == NULL) {
        log_warn(COMP_REPOSITORY, "gk_repository_state_trigger_callback called on NULL repository");
        return;
    }
    repository->callbacks.state_changed_callback(repository);
}

int gk_repository_prepend_repository_path(gk_repository *repository, char *buffer, size_t buffer_length, const char *path, const char *purpose) {
    if (gk_repository_verify(repository, &COMP_REPOSITORY, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    if (buffer == NULL) {
        return gk_repository_failure(repository, &COMP_REPOSITORY, -4, "Error prepending repository path to [%s], destination buffer is NULL", path);
    }

    if (gk_concatenate_paths(buffer, buffer_length, repository->repository_spec.local_path, path) != 0) {
        return gk_repository_failure(repository, &COMP_REPOSITORY, -4, "Error prepending repository path [%s] to [%s] NULL", repository->repository_spec.local_path, path);
    }
    
    return GK_SUCCESS;
}


