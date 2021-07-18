
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "git2.h"

#include "gk_conflicts.h"
#include "gk_session.h"
#include "gk_lg2_private.h"


gk_merge_conflict_entry *gk_merge_conflict_entry_new() {
    gk_merge_conflict_entry *entry = malloc(sizeof(gk_merge_conflict_entry));
    if (entry == NULL) {
        log_error(COMP_CONFLICTS, "Error allocating merge conflict entry");
        return entry;
    }
    memset((void *)entry, 0, sizeof(gk_merge_conflict_entry));
    return entry;
}

void gk_merge_conflict_entry_free(gk_merge_conflict_entry *entry) {
    if (entry == NULL) {
        return;
    }
    free(entry->path);
    entry->path = NULL;
    free(entry);
}

const char *gk_merge_conflict_entry_type_string(gk_merge_conflict_entry_type entry_type) {
    if (entry_type == GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_EDIT) {
        return "incompatible two sided edit";
    }
    else if (entry_type == GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_CREATE) {
        return "incompatible two sided create";
    }
    else if (entry_type == GK_MERGE_CONFLICT_LOCAL_EDIT_REMOTE_DELETE) {
        return "local edit remote delete";
    }
    else  if (entry_type == GK_MERGE_CONFLICT_LOCAL_DELETE_REMOTE_EDIT) {
        return "local delete remote edit";
    }
    log_error(COMP_CONFLICTS, "Unknown conflict type %d, expected one of [GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_EDIT=%d], [GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_CREATE=%d], [GK_MERGE_CONFLICT_LOCAL_DELETE_REMOTE_EDIT=%d] or [GK_MERGE_CONFLICT_LOCAL_EDIT_REMOTE_DELETE=%d]", GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_EDIT, GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_CREATE, GK_MERGE_CONFLICT_LOCAL_DELETE_REMOTE_EDIT, GK_MERGE_CONFLICT_LOCAL_EDIT_REMOTE_DELETE);
    return "unknown conflict entry type";
}
    
int gk_conflicts_allocate(gk_session *session, size_t num_conflicts) {
    session->conflict_summary.num_conflicts = num_conflicts;
    session->conflict_summary.conflicts = (gk_merge_conflict_entry **)calloc(sizeof(gk_merge_conflict_entry *), num_conflicts);
    return 0;
}

void gk_conflicts_free(gk_session *session) {
    for (size_t i = 0; i < session->conflict_summary.num_conflicts; i += 1) {
        //printf("DBG freeing conflict summary at index #%zu (ptr %p)\n", i, (void *)session->conflict_summary.conflicts[i]);
        gk_merge_conflict_entry_free(session->conflict_summary.conflicts[i]);
        session->conflict_summary.conflicts[i] = NULL;
    }
    free(session->conflict_summary.conflicts);
    session->conflict_summary.conflicts = NULL;
}

gk_conflict_diff_summary *gk_conflict_diff_summary_new() {
    gk_conflict_diff_summary *summary = malloc(sizeof(gk_conflict_diff_summary));
    summary->ancestor_to_ours_diff = NULL;
    summary->ancestor_to_theirs_diff = NULL;
    return summary;
}

void gk_conflict_diff_summary_free(gk_conflict_diff_summary *summary) {
    free((char *)summary->ancestor_to_ours_diff);
    free((char *)summary->ancestor_to_theirs_diff);
    summary->ancestor_to_ours_diff = NULL;
    summary->ancestor_to_theirs_diff = NULL;
    free(summary);
}


gk_void_linked_node *gk_void_linked_node_new() {
    gk_void_linked_node *new_node = malloc(sizeof(gk_void_linked_node));
    new_node->data = NULL;
    new_node->next = NULL;
    return new_node;
}

void gk_free_void_node_chain(gk_void_linked_node *chain, int free_data) {
    if (chain == NULL) {
        return;
    }
    gk_free_void_node_chain(chain->next, free_data);
    chain->next = NULL;
    if (free_data == 1) {
        free(chain->data);
    }
    chain->data = NULL;
    free(chain);
}



int gk_conflict_resolve(gk_session *session, const char *path, gk_conflict_resolution accept) {
    if (gk_session_verify(session, &COMP_MERGE, GK_SESSION_VERIFY_LOCAL_CHECKOUT | GK_SESSION_VERIFY_MERGE_IN_PROGRESS, "resolve conflict") != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (path == NULL) {
        return gk_session_failure(session, &COMP_CONFLICTS, -5, "Cannot %s for NULL path", "resolve conflict");
    }
    const char *purpose = "";
    if (accept == GK_CONFLICT_RESOLUTION_OURS) {
        purpose = "resolve conflict (accept ours)";
    }
    else if (accept == GK_CONFLICT_RESOLUTION_THEIRS) {
        purpose = "resolve conflict (accept theirs)";
    }
    else {
        return gk_session_failure(session, &COMP_CONFLICTS, -6, "Canont resolve conflict for path '%s', unknown resolution type '%d' (expected ours '%d' or theirs '%d')", accept, GK_CONFLICT_RESOLUTION_OURS, GK_CONFLICT_RESOLUTION_THEIRS);
    }

    const git_index_entry *conflicted_entry = git_index_get_bypath(session->lg2_resources->merge_index, path, GIT_INDEX_STAGE_NORMAL);
    if (conflicted_entry == NULL) {
        return gk_session_failure(session, &COMP_MERGE, -6, "Cannot %s, file not found in index at path %s", path);
    }

    const git_index_entry *ancestor_entry = NULL;
    const git_index_entry *ours_entry = NULL;
    const git_index_entry *theirs_entry = NULL;

    if (gk_lg2_index_conflict_get(session, &ancestor_entry, &ours_entry, &theirs_entry, session->lg2_resources->merge_index, path, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    git_index_entry clean_entry = *conflicted_entry;

    if (accept == GK_CONFLICT_RESOLUTION_OURS) {
        clean_entry.id = ours_entry->id;
    }
    else if (accept == GK_CONFLICT_RESOLUTION_THEIRS) {
        clean_entry.id = theirs_entry->id;
    }

    if (gk_lg2_index_add(session, &clean_entry, path, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }
            
    if (git_index_conflict_remove(session->lg2_resources->merge_index, path) != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CONFLICTS, -11, "Cannot %s, error removing conflicts for entry '%s' (%d): %s", purpose, path, err->klass, err->message);
    }

    return GK_SUCCESS;
}

int gk_blob_write_contents(gk_session *session, const char *oid_id, const char *path, const char* purpose) {
    if (gk_session_verify(session, &COMP_MERGE, GK_SESSION_VERIFY_LOCAL_CHECKOUT, "write blob") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    git_oid blob_oid;
    if (gk_lg2_oid_from_id(session, &blob_oid, oid_id, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    git_blob *blob = NULL;
    if (gk_lg2_blob_lookup(session, &blob, &blob_oid, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    //int is_binary = git_blob_is_binary(blob);
    git_object_size_t blob_size = git_blob_rawsize(blob);
    const void *blob_data = git_blob_rawcontent(blob);

    FILE *fptr = fopen(path, "w");
    if (fptr == NULL) {
        return gk_session_failure(session, &COMP_CONFLICTS, -10, "Cannot %s, error opening file [%s] for writing: error %d occurred", purpose, path, errno);
    }

    int rc = fwrite(blob_data, 1, blob_size, fptr);
    fclose(fptr);
    if (rc < 0) {
        return gk_session_failure(session, &COMP_CONFLICTS, -10, "Cannot %s, error writing to file [%s]: error %d occurred", purpose, path, rc);
    }
    
    git_blob_free(blob);
    
    return GK_SUCCESS;
}


int gk_conflict_resolve_accept_remote_delete(gk_session *session, const char *path) {
    const char *purpose = "resolve conflict by accepting remote delete";
    if (gk_session_verify(session, &COMP_MERGE, GK_SESSION_VERIFY_LOCAL_CHECKOUT | GK_SESSION_VERIFY_MERGE_IN_PROGRESS, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (path == NULL) {
        return gk_session_failure(session, &COMP_CONFLICTS, -5, "Cannot %s for NULL path", purpose);
    }

    int rc = git_index_remove_bypath(session->lg2_resources->merge_index, path);
    
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CONFLICTS, -3, "Cannot %s, Error removing path '%s' from repository index (%d): %s", purpose, path, err->klass, err->message);
    }

    log_info(COMP_CONFLICTS, "resolved conflict for path [%s] by accepting reomte delete", path);
    return GK_SUCCESS;
}
