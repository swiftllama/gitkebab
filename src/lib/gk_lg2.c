
#include <string.h>

#include "gk_session.h"
#include "gk_lg2_private.h"
#include "gk_logging.h"
#include "gk_conflicts.h"

void gk_lg2_resources_init(gk_session *session) {
    if (session == NULL) {
        log_error(COMP_SESSION, "gk_session_lg2_resources_init called on NULL session");
        return;
    }
    if (session->lg2_resources == NULL) {
        log_error(COMP_SESSION, "gk_session_lg2_resources_init called on session with NULL resources");
        return;
    }
    
    session->lg2_resources->repository_head_ref = NULL;
    session->lg2_resources->repository_head_object = NULL;
    session->lg2_resources->repository_head_commit = NULL;
    session->lg2_resources->repository_head_oid = NULL;
    session->lg2_resources->repository_head_oid_id[0] = '\0';

    session->lg2_resources->fetch_head_ref = NULL;
    session->lg2_resources->fetch_head_object = NULL;
    session->lg2_resources->fetch_head_commit = NULL;
    session->lg2_resources->annotated_fetch_head_commit = NULL;
    session->lg2_resources->fetch_head_oid = NULL;
    session->lg2_resources->fetch_head_oid_id[0] = '\0';

    session->lg2_resources->index = NULL;
    session->lg2_resources->merge_index = NULL;
    session->lg2_resources->tree = NULL;
    session->lg2_resources->reflog = NULL;

    session->lg2_resources->signature = NULL;

    session->lg2_resources->merge_parents = NULL;
    session->lg2_resources->status_list = NULL;
    session->lg2_resources->repository = NULL;
}

void gk_lg2_free_all_but_repository(gk_session *session) {
    if (session == NULL) {
        log_error(COMP_SESSION, "gk_session_lg2_resources_free called on NULL session");
        return;
    }

    gk_lg2_free_references(session);
    gk_lg2_index_free(session);

    gk_lg2_signature_free(session);
    gk_lg2_parents_free(session);
    gk_lg2_tree_free(session);
}

int gk_lg2_load_references(gk_session *session, const char *from_ref_name, const char *purpose) {
    if (session == NULL) {
        log_error(COMP_MERGE, "gk_session_lg2_load_references called on NULL session");
        return GK_FAILURE;
    }

    int rc = git_revparse_ext(&session->lg2_resources->fetch_head_object, &session->lg2_resources->fetch_head_ref, session->lg2_resources->repository, from_ref_name);
    if (rc == GIT_ENOTFOUND) {
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot %s, reference '%s' not found", purpose, from_ref_name);
    }
    if ((rc != 0)) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot %s, the reference '%s' could not be looked up (%d): %s", purpose, from_ref_name, err->klass, err->message);
    }

    rc = git_revparse_ext(&session->lg2_resources->repository_head_object, &session->lg2_resources->repository_head_ref, session->lg2_resources->repository, "HEAD");
    if (rc == GIT_ENOTFOUND) {
        return gk_session_failure(session, &COMP_MERGE, -5, "cannot %s, reference HEAD not found", purpose);
    }
    if ((rc != 0)) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "cannot %s, failed to resolve HEAD reference (%d): %s", purpose, err->klass, err->message);
    }

    rc = git_annotated_commit_from_ref(&session->lg2_resources->annotated_fetch_head_commit, session->lg2_resources->repository, session->lg2_resources->fetch_head_ref);
    if ((rc != 0)) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot %s, error annotating commit for referencd '%s' (%d): %s", purpose, from_ref_name, err->klass, err->message);
    }    

    session->lg2_resources->repository_head_oid = git_object_id(session->lg2_resources->repository_head_object);
    session->lg2_resources->fetch_head_oid = git_object_id(session->lg2_resources->fetch_head_object);
    git_oid_tostr(session->lg2_resources->repository_head_oid_id, 41, session->lg2_resources->repository_head_oid);
    git_oid_tostr(session->lg2_resources->fetch_head_oid_id, 41, session->lg2_resources->fetch_head_oid);

    rc = git_commit_lookup(&session->lg2_resources->repository_head_commit, session->lg2_resources->repository, session->lg2_resources->repository_head_oid);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot %s, error looking up repository head commit [%d] (%d): %s", purpose, session->lg2_resources->repository_head_oid_id, err->klass, err->message);
    }

    rc = git_commit_lookup(&session->lg2_resources->fetch_head_commit, session->lg2_resources->repository, session->lg2_resources->fetch_head_oid);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot %s, error looking up fetch head commit [%d] for remote ref [%s] (%d): %s", purpose, session->lg2_resources->fetch_head_oid_id, from_ref_name, err->klass, err->message);
    }
    
    return GK_SUCCESS;
}

void gk_lg2_free_references(gk_session *session) {
    if (session == NULL) {
        log_error(COMP_SESSION, "gk_session_lg2_resources_free_references called on NULL session");
        return;
    }

    git_reference_free(session->lg2_resources->repository_head_ref);
    git_object_free(session->lg2_resources->repository_head_object);
    git_commit_free(session->lg2_resources->repository_head_commit);
    
    git_reference_free(session->lg2_resources->fetch_head_ref);
    git_object_free(session->lg2_resources->fetch_head_object);
    git_commit_free(session->lg2_resources->fetch_head_commit);
    git_annotated_commit_free(session->lg2_resources->annotated_fetch_head_commit);

    session->lg2_resources->repository_head_ref = NULL;
    session->lg2_resources->repository_head_object = NULL;
    session->lg2_resources->repository_head_commit = NULL;
    session->lg2_resources->fetch_head_ref = NULL;
    session->lg2_resources->fetch_head_object = NULL;
    session->lg2_resources->fetch_head_commit = NULL;
    session->lg2_resources->annotated_fetch_head_commit = NULL;
}

int gk_lg2_index_load(gk_session *session, const char *purpose) {
    if (session == NULL) {
        log_error(COMP_MERGE, "Cannot load resources references for NULL session");
        return GK_FAILURE;
    }

    int rc = git_repository_index(&session->lg2_resources->index, session->lg2_resources->repository);
    if (rc != 0) {
        gk_lg2_index_free(session);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -6, "Error %s, failed to retrieve repository index (%d): %s", purpose, err->klass, err->message);
    }

    return GK_SUCCESS;
}

void gk_lg2_index_free(gk_session *session) {
    if (session->lg2_resources == NULL) {
        return;
    }

    git_index_free(session->lg2_resources->index);
    session->lg2_resources->index = NULL;
}

void gk_lg2_merge_index_free(gk_session *session) {
    if (session->lg2_resources == NULL) {
        return;
    }

    git_index_free(session->lg2_resources->merge_index);
    session->lg2_resources->merge_index = NULL;
}

void gk_lg2_promote_merge_index(gk_session *session) {
    gk_lg2_index_free(session);
    session->lg2_resources->index = session->lg2_resources->merge_index;
    session->lg2_resources->merge_index = NULL;
    
}

int gk_lg2_repository_open(gk_session *session, const char *purpose) {
    int rc = git_repository_open(&session->lg2_resources->repository, session->repository.local_path);
    if (rc != 0) {
        gk_lg2_repository_free(session);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_SESSION, -2, "Failed to %s, repository at local path could not be opened (%d): %s", purpose, err->klass, err->message);
    }
    return GK_SUCCESS;
}

void gk_lg2_repository_free(gk_session *session) {
    git_repository_free(session->lg2_resources->repository);
    session->lg2_resources->repository = NULL;
}

int gk_lg2_reflog_read(gk_session *session, const char *ref_name, const char *purpose) {
    int rc = git_reflog_read(&session->lg2_resources->reflog, session->lg2_resources->repository, ref_name);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_COMMIT, -3, "Cannot %s, error reading reflog for ref '%s' (%d): %s", purpose, ref_name, err->klass, err->message);
    }

    return GK_SUCCESS;
}

void gk_lg2_reflog_free(gk_session *session) {
    git_reflog_free(session->lg2_resources->reflog);
    session->lg2_resources->reflog = NULL;
}

int gk_lg2_parents_lookup(gk_session *session, const char *purpose) {
    session->lg2_resources->merge_parents = calloc(2, sizeof(git_commit *));
    if (session->lg2_resources->repository_head_ref == NULL) {
        return gk_session_failure(session, &COMP_MERGE, -6, "Canot %s, failed looking up parents: repository head is unexpectedly NULL", purpose);
    }

    int rc = git_reference_peel((git_object **)&session->lg2_resources->merge_parents[0], session->lg2_resources->repository_head_ref, GIT_OBJECT_COMMIT);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -7, "Cannot %s, error peeling repository head reference (%d): %s", purpose, err->klass, err->message);
    }

    rc = git_commit_lookup(&session->lg2_resources->merge_parents[1], session->lg2_resources->repository, session->lg2_resources->fetch_head_oid);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot %s, error peeling fetch reference (%d): %s", purpose, err->klass, err->message);
    }

    return GK_SUCCESS;
}

void gk_lg2_parents_free(gk_session *session) {
    if (session->lg2_resources->merge_parents != NULL) {
        git_object_free((git_object *)session->lg2_resources->merge_parents[0]);
        session->lg2_resources->merge_parents[0] = NULL;
        git_commit_free(session->lg2_resources->merge_parents[1]);
        session->lg2_resources->merge_parents[1] = NULL;
    }
    free(session->lg2_resources->merge_parents);
    session->lg2_resources->merge_parents = NULL;
}

int gk_lg2_index_write_tree(gk_session *session, git_index *target_index, const char *purpose) {
    int rc = git_index_write_tree_to(&session->lg2_resources->tree_oid, target_index, session->lg2_resources->repository);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot %s, error writing index tree (%d): %s", purpose, err->klass, err->message);
    }

    rc = git_tree_lookup(&session->lg2_resources->tree, session->lg2_resources->repository, &session->lg2_resources->tree_oid);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -5, "Cannot %s, error looking up tree (%d): %s", purpose, err->klass, err->message);
    }

    git_oid_tostr(session->lg2_resources->tree_oid_id, 41, &session->lg2_resources->tree_oid);
    log_info(COMP_MERGE, "while [%s], Wrote tree [%s]", purpose, session->lg2_resources->tree_oid_id);

    return GK_SUCCESS;
}

void gk_lg2_tree_free(gk_session *session) {
    git_tree_free(session->lg2_resources->tree);
    session->lg2_resources->tree = NULL;
}

int gk_lg2_signature_create(gk_session *session, log_Component *component, const char *purpose) {
    int rc = git_signature_default(&session->lg2_resources->signature, session->lg2_resources->repository);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, component, -3, "Cannot %s, error creating signature (%d): %s", purpose, err->klass, err->message);
    }

    return GK_SUCCESS;
}

void gk_lg2_signature_free(gk_session *session) {
    git_signature_free(session->lg2_resources->signature);
    session->lg2_resources->signature = NULL;
}

void gk_lg2_status_list_free(gk_session *session) {
    git_status_list_free(session->lg2_resources->status_list);
    session->lg2_resources->status_list = NULL;
}

int gk_lg2_status_list_load(gk_session *session, const char *purpose) {
    git_status_options status_options = GIT_STATUS_OPTIONS_INIT;
    
    status_options.show  = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    status_options.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
        GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS |
        GIT_STATUS_OPT_RENAMES_INDEX_TO_WORKDIR | 
        GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
        GIT_STATUS_OPT_SORT_CASE_INSENSITIVELY;
    
    int rc = git_status_list_new(&session->lg2_resources->status_list, session->lg2_resources->repository, &status_options);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_STATUS, -2, "Cannot %s, failed to  query status (%d): %s", purpose, err->klass, err->message);
    }

    return GK_SUCCESS;
}

int gk_lg2_checkout_tree(gk_session *session, git_checkout_options *checkout_options, const char *purpose) {
    int rc = git_checkout_tree(session->lg2_resources->repository, (git_object *)session->lg2_resources->tree, checkout_options);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -6, "Cannot %s, error checking out tree [%s] (%d): %s", purpose, session->lg2_resources->tree_oid_id, err->klass, err->message);
    }

    return GK_SUCCESS;
}

int gk_lg2_iterate_conflicts(gk_session *session, const char *purpose) {
    log_info(COMP_CONFLICTS, "Iterating over conflicts");
    git_index_conflict_iterator *conflicts = NULL;

    gk_void_linked_node *conflict_chain = gk_void_linked_node_new();
    gk_void_linked_node *next_conflict_node = conflict_chain;
        
    int rc = git_index_conflict_iterator_new(&conflicts, session->lg2_resources->merge_index);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -6, "Cannot %s, error getting conflict iterator (%d): %s", purpose, err->klass, err->message);
        git_index_conflict_iterator_free(conflicts);
    }

    int index = 0;
    size_t num_conflicts = 0;
    gk_lg2_conflict_entry conflict_entry;
    gk_lg2_conflict_entry_init(&conflict_entry);
    
    while ((rc = git_index_conflict_next(&conflict_entry.ancestor, &conflict_entry.ours, &conflict_entry.theirs, conflicts)) == 0) {
        const char *entry_path = "";
        if (conflict_entry.ours != NULL) {
            git_oid_tostr(conflict_entry.ours_oid_id, 41, &conflict_entry.ours->id);
            if (gk_lg2_blob_lookup(session, &conflict_entry.ours_blob, &conflict_entry.ours->id, purpose) != GK_SUCCESS) {
                gk_lg2_conflict_entry_free_members(&conflict_entry);
                git_index_conflict_iterator_free(conflicts);
                gk_free_void_node_chain(conflict_chain, 1);
                return GK_FAILURE;
            }
            entry_path = conflict_entry.ours->path;
        }
        if (conflict_entry.theirs != NULL) {
            git_oid_tostr(conflict_entry.theirs_oid_id, 41, &conflict_entry.theirs->id);
            if (gk_lg2_blob_lookup(session, &conflict_entry.theirs_blob, &conflict_entry.theirs->id, purpose) != GK_SUCCESS) {
                gk_lg2_conflict_entry_free_members(&conflict_entry);
                git_index_conflict_iterator_free(conflicts);
                gk_free_void_node_chain(conflict_chain, 1);
                return GK_FAILURE;
            }
            entry_path = conflict_entry.theirs->path;
        }
        if (conflict_entry.ancestor != NULL) {
            git_oid_tostr(conflict_entry.ancestor_oid_id, 41, &conflict_entry.ancestor->id);
            if (gk_lg2_blob_lookup(session, &conflict_entry.ancestor_blob, &conflict_entry.ancestor->id,  purpose) != GK_SUCCESS) {
                gk_lg2_conflict_entry_free_members(&conflict_entry);
                git_index_conflict_iterator_free(conflicts);
                gk_free_void_node_chain(conflict_chain, 1);
                return GK_FAILURE;
            }
            entry_path = conflict_entry.ancestor->path;
        }
        gk_merge_conflict_entry *ext_entry = gk_merge_conflict_entry_new();

        ext_entry->path = strdup(entry_path);
        ext_entry->conflict_type = GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_EDIT;
        if (conflict_entry.ancestor != NULL) {
            if (conflict_entry.ours == NULL) {
                ext_entry->conflict_type = GK_MERGE_CONFLICT_LOCAL_DELETE_REMOTE_EDIT;
            }
            else if (conflict_entry.theirs == NULL) {
                ext_entry->conflict_type = GK_MERGE_CONFLICT_LOCAL_EDIT_REMOTE_DELETE;
            }
        }
        else {
            ext_entry->conflict_type = GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_CREATE;
        }
        
        strncpy(ext_entry->ancestor_oid_id, conflict_entry.ancestor_oid_id, 41);
        strncpy(ext_entry->ours_oid_id, conflict_entry.ours_oid_id, 41);
        strncpy(ext_entry->theirs_oid_id, conflict_entry.theirs_oid_id, 41);

        next_conflict_node->data = (void *)ext_entry;
        next_conflict_node->next = gk_void_linked_node_new();
        next_conflict_node = next_conflict_node->next;

        log_info(COMP_CONFLICTS, "Conflict #%d at file [%s], of type [%s] has ancestor [%s], ours [%s], theirs [%s]", index, entry_path, gk_merge_conflict_entry_type_string(ext_entry->conflict_type), ext_entry->ancestor_oid_id, ext_entry->ours_oid_id, ext_entry->theirs_oid_id);
        
        // DBG
        /*
        gk_conflict_diff_summary *summary = NULL;
        summary = gk_lg2_conflict_diff_summary(session, entry, purpose);

        log_error(COMP_CONFLICTS, "DBG C0x found ancestor-to-ours diff: -------\n%s\n---------", summary->ancestor_to_ours_diff);
        log_error(COMP_CONFLICTS, "DBG C1x found ancestor-to-theirs diff: -------\n%s\n---------", summary->ancestor_to_theirs_diff);

        gk_conflict_diff_summary_free(summary);*/
        // END DBG


        
        gk_lg2_conflict_entry_free_members(&conflict_entry);
        index += 1;
        num_conflicts += 1;
    }

    log_info(COMP_MERGE, "Done iteratoring over conflicts (err: %d)", (rc != GIT_ITEROVER ? rc : 0));
    
    if (rc != GIT_ITEROVER) {
        git_index_conflict_iterator_free(conflicts);
        gk_free_void_node_chain(conflict_chain, 1);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_MERGE, -6, "Cannot %s, error getting next conflict from iterator (%d): %s", purpose, err->klass, err->message);
    }

    gk_conflicts_free(session);
    gk_conflicts_allocate(session, num_conflicts);
    next_conflict_node = conflict_chain;
    index = 0;
    while((next_conflict_node != NULL) && (next_conflict_node->data != NULL)) {
        session->conflict_summary.conflicts[index] = next_conflict_node->data;
        next_conflict_node = next_conflict_node->next;
        index += 1;
    }
    
    gk_free_void_node_chain(conflict_chain, 0);
    git_index_conflict_iterator_free(conflicts);
    return GK_SUCCESS;
}

void gk_lg2_conflict_entry_init(gk_lg2_conflict_entry *entry) {
    entry->ancestor = NULL;
    entry->ours = NULL;
    entry->theirs = NULL;
    entry->ancestor_blob = NULL;
    entry->ours_blob = NULL;
    entry->theirs_blob = NULL;
    entry->ancestor_oid_id[0] = '\0';
    entry->ours_oid_id[0] = '\0';
    entry->theirs_oid_id[0] = '\0';
}

void gk_lg2_conflict_entry_free_members(gk_lg2_conflict_entry *entry) {
    git_blob_free(entry->ancestor_blob);
    git_blob_free(entry->ours_blob);
    git_blob_free(entry->theirs_blob);
    entry->ancestor = NULL;
    entry->ours = NULL;
    entry->theirs = NULL;
    entry->ancestor_blob = NULL;
    entry->ours_blob = NULL;
    entry->theirs_blob = NULL;
    entry->ancestor_oid_id[0] = '\0';
    entry->ours_oid_id[0] = '\0';
    entry->theirs_oid_id[0] = '\0';
}

gk_conflict_diff_summary *gk_lg2_conflict_diff_summary(gk_session *session, gk_merge_conflict_entry *entry, const char *purpose) {
    if (session == NULL) {
        log_error(COMP_CONFLICTS, "Cannot calculate conflict diff summary for NULL session");
        return NULL;
    }
    if (entry == NULL) {
        log_error(COMP_CONFLICTS, "Cannot calculate conflict diff summary for NULL entry");
        return NULL;
    }

    git_oid ancestor_oid;
    git_oid ours_oid;
    git_oid theirs_oid;

    git_blob *ancestor_blob = NULL;
    git_blob *ours_blob = NULL;
    git_blob *theirs_blob = NULL;

    if (gk_lg2_oid_from_id(session, &ancestor_oid, entry->ancestor_oid_id, "retrieve ancestor oid for conflict summary") != GK_SUCCESS) {
        return NULL;
    }

    if (gk_lg2_oid_from_id(session, &ours_oid, entry->ours_oid_id, "retrieve ours oid for conflict summary") != GK_SUCCESS) {
        return NULL;
    }

    if (gk_lg2_oid_from_id(session, &theirs_oid, entry->theirs_oid_id, "retrieve theirs oid for conflict summary") != GK_SUCCESS) {
        return NULL;
    }

    if (gk_lg2_blob_lookup(session, &ancestor_blob, &ancestor_oid, purpose) != GK_SUCCESS) {
        return NULL;
    }

    if (gk_lg2_blob_lookup(session, &ours_blob, &ours_oid, purpose) != GK_SUCCESS) {
        return NULL;
    }

    if (gk_lg2_blob_lookup(session, &theirs_blob, &theirs_oid, purpose) != GK_SUCCESS) {
        return NULL;
    }
    
    git_patch *ancestor_to_ours_patch = NULL;
    git_patch *ancestor_to_theirs_patch = NULL;

    git_buf ancestor_to_ours_buf = {0};
    git_buf ancestor_to_theirs_buf = {0};

    git_diff_options diff_options = GIT_DIFF_OPTIONS_INIT;
        
    gk_conflict_diff_summary *summary = gk_conflict_diff_summary_new();
    
    int rc = git_patch_from_blobs(&ancestor_to_ours_patch, ancestor_blob, entry->path, ours_blob, entry->path, &diff_options);
    if (rc != 0) {
        git_patch_free(ancestor_to_ours_patch);
        const git_error *err = git_error_last();
        gk_session_failure(session, &COMP_CONFLICTS, -6, "Cannot %s, error generating ancestor to ours patch for conflict (%d): %s", purpose, err->klass, err->message);
        return NULL;
    }

    rc = git_patch_from_blobs(&ancestor_to_theirs_patch, ancestor_blob, entry->path, theirs_blob, entry->path, &diff_options);
    if (rc != 0) {
        git_patch_free(ancestor_to_theirs_patch);
        git_patch_free(ancestor_to_ours_patch);
        const git_error *err = git_error_last();
        gk_session_failure(session, &COMP_CONFLICTS, -6, "Cannot %s, error generating ancestor to theirs patch for conflict at (%d): %s", purpose, err->klass, err->message);
        return NULL;
    }

    rc = git_patch_to_buf(&ancestor_to_ours_buf, ancestor_to_ours_patch);
    if (rc != 0) {
        git_patch_free(ancestor_to_theirs_patch);
        git_patch_free(ancestor_to_ours_patch);
        git_buf_dispose(&ancestor_to_ours_buf);
        const git_error *err = git_error_last();
        gk_session_failure(session, &COMP_CONFLICTS, -6, "Cannot %s, error getting buf for ancestor to ours patch for conflictd (%d): %s", purpose, index, err->klass, err->message);
        return NULL;
    }

    rc = git_patch_to_buf(&ancestor_to_theirs_buf, ancestor_to_theirs_patch);
    if (rc != 0) {
        git_patch_free(ancestor_to_theirs_patch);
        git_patch_free(ancestor_to_ours_patch);
        git_buf_dispose(&ancestor_to_ours_buf);
        git_buf_dispose(&ancestor_to_theirs_buf);
        const git_error *err = git_error_last();
        gk_session_failure(session, &COMP_CONFLICTS, -6, "Cannot %s, error getting buf for ancestor to theis patch for conflict (%d): %s", purpose, index, err->klass, err->message);
        return NULL;
    }

    summary->ancestor_to_ours_diff = strdup(ancestor_to_ours_buf.ptr);
    summary->ancestor_to_theirs_diff = strdup(ancestor_to_theirs_buf.ptr);

    return summary;
}

int gk_lg2_index_conflict_get(gk_session *session, const git_index_entry **ancestor_entry, const git_index_entry **ours_entry, const git_index_entry **theirs_entry, git_index *index, const char *path, const char *purpose) {
    int rc = git_index_conflict_get(ancestor_entry, ours_entry, theirs_entry, index, path);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CONFLICTS, -10, "Cannot %s, error obtaining conflict entries for path '%s' (%d): %s", purpose, path, err->klass, err->message);
    }
    return GK_SUCCESS;
}

int gk_lg2_index_add(gk_session *session, const git_index_entry *entry, const char *path, const char *purpose) {
    int rc = git_index_add(session->lg2_resources->merge_index, entry);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CONFLICTS, -10, "Cannot %s, error adding entry for path '%s' (%d): %s", purpose, path, err->klass, err->message);
    }
    return GK_SUCCESS;
}

int gk_lg2_oid_from_id(gk_session *session, git_oid *oid, const char *oid_id, const char *purpose) {
    int rc = git_oid_fromstr(oid, oid_id);
    if (rc != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CONFLICTS, -6, "Cannot %s, error finding oid corresponding to id '%s' (%d): %s", purpose, oid_id, err->klass, err->message);
    }
    return GK_SUCCESS;
}

int gk_lg2_blob_lookup(gk_session *session, git_blob **blob, const git_oid *oid, const char *purpose) {
    int rc = git_blob_lookup(blob, session->lg2_resources->repository, oid);
    if (rc != 0) {
        if (blob != NULL) {
            git_blob_free(*blob);
        }
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CONFLICTS, -6, "Cannot %s, error finding blob corresponding to id '%s' (%d): %s", purpose, git_oid_tostr_s(oid), err->klass, err->message);
    }
    return GK_SUCCESS;
}
