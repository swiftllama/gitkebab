
#include <string.h>

#include "gk_repository.h"
#include "gk_lg2_private.h"
#include "gk_logging.h"
#include "gk_conflicts.h"
#include "gk_filesystem.h"
#include "gk_session.h"

void gk_lg2_resources_init(gk_repository *repository) {
    if (repository == NULL) {
        log_error(COMP_REPOSITORY, "gk_repository_lg2_resources_init called on NULL repository");
        return;
    }
    if (repository->lg2_resources == NULL) {
        log_error(COMP_REPOSITORY, "gk_repository_lg2_resources_init called on repository with NULL resources");
        return;
    }

    gk_lg2_resources *lg2_resources = repository->lg2_resources;
    lg2_resources->repository_head_ref = NULL;
    lg2_resources->repository_head_object = NULL;
    lg2_resources->repository_head_commit = NULL;
    lg2_resources->repository_head_oid = NULL;
    lg2_resources->repository_head_oid_id[0] = '\0';

    lg2_resources->fetch_head_ref = NULL;
    lg2_resources->fetch_head_object = NULL;
    lg2_resources->fetch_head_commit = NULL;
    lg2_resources->annotated_fetch_head_commit = NULL;
    lg2_resources->fetch_head_oid = NULL;
    lg2_resources->fetch_head_oid_id[0] = '\0';

    lg2_resources->index = NULL;
    lg2_resources->merge_index = NULL;
    lg2_resources->tree = NULL;
    lg2_resources->reflog = NULL;

    lg2_resources->signature = NULL;

    lg2_resources->merge_parents = NULL;
    lg2_resources->status_list = NULL;
    lg2_resources->repository = NULL;
}

void gk_lg2_free_all_but_repository(gk_repository *repository) {
    if (repository == NULL) {
        log_error(COMP_REPOSITORY, "gk_repository_lg2_resources_free called on NULL repository");
        return;
    }

    gk_lg2_free_references(repository);
    gk_lg2_index_free(repository);

    gk_lg2_signature_free(repository);
    gk_lg2_parents_free(repository);
    gk_lg2_tree_free(repository);
}

int gk_lg2_load_references(gk_session *session) {
    const char *purpose = "load index and head references";
    if (gk_session_context_push(session, purpose, NULL, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return 0;
    }

    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    if (lg2_resources->index != NULL) {
        gk_lg2_free_references(session->repository);
    }

    const char *from_ref_name = session->repository->spec.remote_ref_name;
    int rc = git_revparse_ext(&lg2_resources->fetch_head_object, &lg2_resources->fetch_head_ref, lg2_resources->repository, from_ref_name);
    if (rc == GIT_ENOTFOUND) {
        //return gk_session_failure_ex(session, purpose, GK_ERR, "could not revparse refname [%s], ref not found", from_ref_name);
        log_info(COMP_REPOSITORY, "repository does not contain a refspec [%s], might be empty repository", from_ref_name);
        lg2_resources->fetch_head_object = NULL;
        lg2_resources->fetch_head_ref = NULL;
    }
    else if ((rc != 0)) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "could not revparse refname [%s]", from_ref_name);
    }

    rc = git_revparse_ext(&lg2_resources->repository_head_object, &lg2_resources->repository_head_ref, lg2_resources->repository, "HEAD");
    if (rc == GIT_ENOTFOUND) {
        log_info(COMP_REPOSITORY, "repository does not contain a HEAD refname, might be empty repository");
        lg2_resources->repository_head_object = NULL;
        lg2_resources->repository_head_ref = NULL;
    }
    else if ((rc != 0)) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "could not revparse refname HEAD");
    }

    if (lg2_resources->repository_head_object != NULL) {
        lg2_resources->repository_head_oid = git_object_id(lg2_resources->repository_head_object);
        git_oid_tostr(lg2_resources->repository_head_oid_id, 41, lg2_resources->repository_head_oid);


        if (git_commit_lookup(&lg2_resources->repository_head_commit, lg2_resources->repository, lg2_resources->repository_head_oid) != 0) {
            return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "Could not look up HEAD commit [%s]", lg2_resources->repository_head_oid_id);
        }
    }

    if (lg2_resources->fetch_head_object != NULL) {
        rc = git_annotated_commit_from_ref(&lg2_resources->annotated_fetch_head_commit, lg2_resources->repository, lg2_resources->fetch_head_ref);
        if ((rc != 0)) {
            return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "could not annotate commit for reference [%s]", from_ref_name);
        }
        
        lg2_resources->fetch_head_oid = git_object_id(lg2_resources->fetch_head_object);
        git_oid_tostr(lg2_resources->fetch_head_oid_id, 41, lg2_resources->fetch_head_oid);

        if (git_commit_lookup(&lg2_resources->fetch_head_commit, lg2_resources->repository, lg2_resources->fetch_head_oid) != 0) {
            return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "Could not look up FETCH HEAD  commit [%s] for reference [%s]", lg2_resources->fetch_head_oid_id, from_ref_name);
        }
    }
    
    return gk_session_success(session, purpose);
}

void gk_lg2_free_references(gk_repository *repository) {
    if (repository == NULL) {
        log_error(COMP_REPOSITORY, "gk_lg2_free_references called on NULL repository");
        return;
    }

    gk_lg2_resources *lg2_resources = repository->lg2_resources;
    git_reference_free(lg2_resources->repository_head_ref);
    git_object_free(lg2_resources->repository_head_object);
    git_commit_free(lg2_resources->repository_head_commit);
    
    git_reference_free(lg2_resources->fetch_head_ref);
    git_object_free(lg2_resources->fetch_head_object);
    git_commit_free(lg2_resources->fetch_head_commit);
    git_annotated_commit_free(lg2_resources->annotated_fetch_head_commit);

    lg2_resources->repository_head_ref = NULL;
    lg2_resources->repository_head_object = NULL;
    lg2_resources->repository_head_commit = NULL;
    lg2_resources->fetch_head_ref = NULL;
    lg2_resources->fetch_head_object = NULL;
    lg2_resources->fetch_head_commit = NULL;
    lg2_resources->annotated_fetch_head_commit = NULL;
}

int gk_lg2_index_load(gk_session *session) {
    const char *purpose = "load repository index";
    if (gk_session_context_push(session, purpose, NULL, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return 0;
    }

    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    if (git_repository_index(&lg2_resources->index, lg2_resources->repository) != 0) {
        gk_lg2_index_free(session->repository);
        return gk_session_lg2_failure(session, purpose, GK_ERR);
    }

    return gk_session_success(session, purpose);
}

void gk_lg2_index_free(gk_repository *repository) {
    git_index_free(repository->lg2_resources->index);
    repository->lg2_resources->index = NULL;
}

void gk_lg2_merge_index_free(gk_repository *repository) {
    git_index_free(repository->lg2_resources->merge_index);
    repository->lg2_resources->merge_index = NULL;
}

int gk_lg2_promote_merge_index(gk_session *session) {
    const char *purpose = "promote merge index";
    if (gk_session_context_push(session, purpose, NULL, GK_REPOSITORY_VERIFY_MERGE_INDEX_LOADED) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
    
    gk_lg2_index_free(session->repository);
    lg2_resources->index = lg2_resources->merge_index;
    lg2_resources->merge_index = NULL;

    checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE | GIT_CHECKOUT_ALLOW_CONFLICTS;
    checkout_options.progress_cb = gk_session_checkout_progress_callback;
    checkout_options.progress_payload = session;
    
    if (gk_lg2_index_write_tree(session, lg2_resources->index) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }
        
    if (gk_lg2_checkout_tree(session, &checkout_options) != GK_SUCCESS) {
        return gk_session_failure(session, purpose);
    }

    return gk_session_success(session, purpose);
}

int gk_lg2_repository_open(gk_session *session) {
    const char *purpose = "open repository";
    if (gk_session_context_push(session, purpose, &COMP_REPOSITORY, GK_REPOSITORY_VERIFY_NONE) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    if (git_repository_open(&lg2_resources->repository, session->repository->spec.local_path) != 0) {
        gk_lg2_repository_free(session->repository);
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "repository at local path [%s] could not be opened", session->repository->spec.local_path);
    }
    return gk_session_success(session, purpose);
}

int gk_lg2_repository_init_ext(gk_session *session, const char* main_branch_name) {
    const char *purpose = "libgit2 repository init";
    if (gk_session_context_push(session, purpose, &COMP_REPOSITORY, GK_REPOSITORY_VERIFY_NONE) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;

    git_repository_init_options opts;
    git_repository_init_options_init(&opts, GIT_REPOSITORY_INIT_OPTIONS_VERSION);
    opts.initial_head = main_branch_name;
    opts.flags = GIT_REPOSITORY_INIT_MKPATH;
    
    if (git_repository_init_ext(&lg2_resources->repository, session->repository->spec.local_path, &opts) != 0) {
        gk_lg2_repository_free(session->repository);
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "Could not init repository at local path [%s] with main branch [%s]", session->repository->spec.local_path, main_branch_name);
    }
    return gk_session_success(session, purpose);
}

void gk_lg2_repository_free(gk_repository *repository) {
    git_repository_free(repository->lg2_resources->repository);
    repository->lg2_resources->repository = NULL;
}

int gk_lg2_reflog_read(gk_session *session, const char *ref_name) {
    const char *purpose = "read reflog";
    if (gk_session_context_push(session, purpose, &COMP_COMMIT, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return 0;
    }
    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    int rc = git_reflog_read(&lg2_resources->reflog, lg2_resources->repository, ref_name);
    if (rc != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to read reflog for ref [%s]", ref_name);
    }

    return gk_session_success(session, purpose);
}

void gk_lg2_reflog_free(gk_repository *repository) {
    git_reflog_free(repository->lg2_resources->reflog);
    repository->lg2_resources->reflog = NULL;
}

int gk_lg2_parents_lookup(gk_session *session) {
    const char *purpose = "lookup commit parents";
    if (gk_session_context_push(session, purpose, &COMP_MERGE, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    lg2_resources->merge_parents = calloc(2, sizeof(git_commit *));
    if (lg2_resources->repository_head_ref == NULL) {
        return gk_session_failure_ex(session, purpose, GK_ERR, "repository head is unexpectedly NULL");
    }

    if (git_reference_peel((git_object **)&lg2_resources->merge_parents[0], lg2_resources->repository_head_ref, GIT_OBJECT_COMMIT) != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "error peeling repository HEAD reference");
    }

    if(git_commit_lookup(&lg2_resources->merge_parents[1], lg2_resources->repository, lg2_resources->fetch_head_oid) != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "error peeing fetch HEAD reference");
    }

    return gk_session_success(session, purpose);
}

void gk_lg2_parents_free(gk_repository *repository) {
    gk_lg2_resources *lg2_resources = repository->lg2_resources;
    if (lg2_resources->merge_parents != NULL) {
        git_object_free((git_object *)lg2_resources->merge_parents[0]);
        lg2_resources->merge_parents[0] = NULL;
        git_commit_free(lg2_resources->merge_parents[1]);
        lg2_resources->merge_parents[1] = NULL;
    }
    free(lg2_resources->merge_parents);
    lg2_resources->merge_parents = NULL;
}

int gk_lg2_index_write_tree(gk_session *session, git_index *target_index) {
    const char *purpose = "write index tree";
    if (gk_session_context_push(session, purpose, NULL, GK_REPOSITORY_VERIFY_INDEX_LOADED) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    if (git_index_write_tree_to(&lg2_resources->tree_oid, target_index, lg2_resources->repository) != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "error writing index tree");
    }

    git_oid_tostr(lg2_resources->tree_oid_id, 41, &lg2_resources->tree_oid);
    if (git_tree_lookup(&lg2_resources->tree, lg2_resources->repository, &lg2_resources->tree_oid) != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "error looking up index tree [%s]", lg2_resources->tree_oid_id);
    }

    log_info(COMP_MERGE, "while [%s], Wrote tree [%s]", purpose, lg2_resources->tree_oid_id);
    return gk_session_success(session, purpose);
}
                            
void gk_lg2_tree_free(gk_repository *repository) {
    git_tree_free(repository->lg2_resources->tree);
    repository->lg2_resources->tree = NULL;
}

int gk_lg2_signature_create(gk_session *session) {
    const char *purpose = "create signature";
    if (gk_session_context_push(session, purpose, NULL, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    if (git_signature_now(&lg2_resources->signature, "gitkebab", "info@example.com") != 0) {
        return gk_session_lg2_failure(session, purpose, GK_ERR);
    }

    return gk_session_success(session, purpose);
}

void gk_lg2_signature_free(gk_repository *repository) {
    git_signature_free(repository->lg2_resources->signature);
    repository->lg2_resources->signature = NULL;
}

void gk_lg2_status_list_free(gk_repository *repository) {
    git_status_list_free(repository->lg2_resources->status_list);
    repository->lg2_resources->status_list = NULL;
}

int gk_lg2_status_list_load(gk_session *session) {
    const char *purpose = "load status list";
    if (gk_session_context_push(session, purpose, NULL, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    git_status_options status_options = GIT_STATUS_OPTIONS_INIT;
    
    status_options.show  = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    status_options.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
        GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS |
        GIT_STATUS_OPT_RENAMES_INDEX_TO_WORKDIR | 
        GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
        GIT_STATUS_OPT_SORT_CASE_INSENSITIVELY;
    
    if(git_status_list_new(&lg2_resources->status_list, lg2_resources->repository, &status_options) != 0) {
        return gk_session_lg2_failure(session, purpose, GK_ERR);
    }

    return gk_session_success(session, purpose);
}

int gk_lg2_checkout_tree(gk_session *session, git_checkout_options *checkout_options) {
    const char *purpose = "checkout tree";
    if (gk_session_context_push(session, purpose, NULL, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    if (git_checkout_tree(lg2_resources->repository, (git_object *)lg2_resources->tree, checkout_options) != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "error checking out tree [%s]", lg2_resources->tree_oid_id);
    }

    return gk_session_success(session, purpose);
}

int gk_lg2_iterate_conflicts(gk_session *session) {
    const char *purpose = "load conflicts";
    if (gk_session_context_push(session, purpose, &COMP_CONFLICTS, GK_REPOSITORY_VERIFY_MERGE_INDEX_LOADED) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    gk_lg2_resources *lg2_resources = session->repository->lg2_resources;
    git_index_conflict_iterator *conflicts = NULL;

    log_info(COMP_CONFLICTS, "Iterating over conflicts");

    gk_void_linked_node *conflict_chain = gk_void_linked_node_new();
    gk_void_linked_node *next_conflict_node = conflict_chain;
        
    if (git_index_conflict_iterator_new(&conflicts, lg2_resources->merge_index) != 0) {
        git_index_conflict_iterator_free(conflicts);
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to get conflict iterator");
    }

    int index = 0;
    size_t num_conflicts = 0;
    gk_lg2_conflict_entry conflict_entry;
    gk_lg2_conflict_entry_init(&conflict_entry);

    int rc;
    while ((rc = git_index_conflict_next(&conflict_entry.ancestor, &conflict_entry.ours, &conflict_entry.theirs, conflicts)) == 0) {
        const char *entry_path = "";
        if (conflict_entry.ours != NULL) {
            git_oid_tostr(conflict_entry.ours_oid_id, 41, &conflict_entry.ours->id);
            if (gk_lg2_blob_lookup(session, &conflict_entry.ours_blob, &conflict_entry.ours->id) != GK_SUCCESS) {
                gk_lg2_conflict_entry_free_members(&conflict_entry);
                git_index_conflict_iterator_free(conflicts);
                gk_free_void_node_chain(conflict_chain, 1);
                return gk_session_failure(session, purpose);
            }
            entry_path = conflict_entry.ours->path;
        }
        if (conflict_entry.theirs != NULL) {
            git_oid_tostr(conflict_entry.theirs_oid_id, 41, &conflict_entry.theirs->id);
            if (gk_lg2_blob_lookup(session, &conflict_entry.theirs_blob, &conflict_entry.theirs->id) != GK_SUCCESS) {
                gk_lg2_conflict_entry_free_members(&conflict_entry);
                git_index_conflict_iterator_free(conflicts);
                gk_free_void_node_chain(conflict_chain, 1);
                return gk_session_failure(session, purpose);
            }
            entry_path = conflict_entry.theirs->path;
        }
        if (conflict_entry.ancestor != NULL) {
            git_oid_tostr(conflict_entry.ancestor_oid_id, 41, &conflict_entry.ancestor->id);
            if (gk_lg2_blob_lookup(session, &conflict_entry.ancestor_blob, &conflict_entry.ancestor->id) != GK_SUCCESS) {
                gk_lg2_conflict_entry_free_members(&conflict_entry);
                git_index_conflict_iterator_free(conflicts);
                gk_free_void_node_chain(conflict_chain, 1);
                return gk_session_failure(session, purpose);
            }
            entry_path = conflict_entry.ancestor->path;
        }
        gk_merge_conflict_entry *ext_entry = gk_merge_conflict_entry_new();

        ext_entry->path = strdup(entry_path);
        ext_entry->conflict_type = GK_MERGE_CONFLICT_INCOMPATIBLE_TWOSIDED_EDIT;
        if (conflict_entry.ancestor != NULL) {
            // NOTE: the deleted file may have been replaced by a
            //       folder with the same name. We can detect this by
            //       looking for entries in the index that start with
            //       "<entry_path>/", since git doesn't track folders
            //       but if there is one then it must contain at least
            //       one file (or subfolder with file, etc). This can
            //       be done using git_index_find_prefix(..)
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
        
        gk_lg2_conflict_entry_free_members(&conflict_entry);
        index += 1;
        num_conflicts += 1;
    }

    log_info(COMP_CONFLICTS, "Done iteratoring over [%zu] conflicts (err: %d)", num_conflicts, (rc != GIT_ITEROVER ? rc : 0));
    
    if (rc != GIT_ITEROVER) {
        git_index_conflict_iterator_free(conflicts);
        gk_free_void_node_chain(conflict_chain, 1);
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to get next conflict from iterator");
    }

    gk_repository *repository = session->repository;
    gk_conflicts_free(session->repository);
    gk_conflicts_allocate(session->repository, num_conflicts);
    next_conflict_node = conflict_chain;
    strncpy(repository->conflict_summary.repository_head_oid_id, lg2_resources->repository_head_oid_id, 41);
    strncpy(repository->conflict_summary.fetch_head_oid_id, lg2_resources->fetch_head_oid_id, 41);
    index = 0;
    log_info(COMP_CONFLICTS, "Merge status from fetch head [%s] into repository head [%s] has %zu conflicts", repository->conflict_summary.fetch_head_oid_id, repository->conflict_summary.repository_head_oid_id, repository->conflict_summary.num_conflicts);
    while((next_conflict_node != NULL) && (next_conflict_node->data != NULL)) {
        gk_merge_conflict_entry *entry = next_conflict_node->data;
        log_info(COMP_CONFLICTS, "Conflict #%d at file [%s], of type [%s] has ancestor [%s], ours [%s], theirs [%s]", index, entry->path, gk_merge_conflict_entry_type_string(entry->conflict_type), entry->ancestor_oid_id, entry->ours_oid_id, entry->theirs_oid_id);
        repository->conflict_summary.conflicts[index] = entry;
        next_conflict_node = next_conflict_node->next;
        index += 1;
    }

    gk_free_void_node_chain(conflict_chain, 0);
    git_index_conflict_iterator_free(conflicts);
    return gk_session_success(session, purpose);
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

gk_conflict_diff_summary *gk_lg2_conflict_diff_summary(gk_session *session, gk_merge_conflict_entry *entry) {
    const char *purpose = "summarize diff";
    if (gk_session_context_push(session, purpose, NULL, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return NULL;
    }
    if (entry == NULL) {
        log_error(COMP_CONFLICTS, "Cannot calculate conflict diff summary for NULL entry");
        gk_session_failure_ex(session, purpose, GK_ERR, "entry is NULL");
        return NULL;
    }

    git_oid ancestor_oid;
    git_oid ours_oid;
    git_oid theirs_oid;

    git_blob *ancestor_blob = NULL;
    git_blob *ours_blob = NULL;
    git_blob *theirs_blob = NULL;

    if (gk_lg2_oid_from_id(session, &ancestor_oid, entry->ancestor_oid_id) != GK_SUCCESS) {
        gk_session_failure(session, purpose);
        return NULL;
    }

    if (gk_lg2_oid_from_id(session, &ours_oid, entry->ours_oid_id) != GK_SUCCESS) {
        gk_session_failure(session, purpose);
        return NULL;
    }

    if (gk_lg2_oid_from_id(session, &theirs_oid, entry->theirs_oid_id) != GK_SUCCESS) {
        gk_session_failure(session, purpose);
        return NULL;
    }

    if (gk_lg2_blob_lookup(session, &ancestor_blob, &ancestor_oid) != GK_SUCCESS) {
        gk_session_failure(session, purpose);
        return NULL;
    }

    if (gk_lg2_blob_lookup(session, &ours_blob, &ours_oid) != GK_SUCCESS) {
        gk_session_failure(session, purpose);
        return NULL;
    }

    if (gk_lg2_blob_lookup(session, &theirs_blob, &theirs_oid) != GK_SUCCESS) {
        gk_session_failure(session, purpose);
        return NULL;
    }
    
    git_patch *ancestor_to_ours_patch = NULL;
    git_patch *ancestor_to_theirs_patch = NULL;

    git_buf ancestor_to_ours_buf = {0};
    git_buf ancestor_to_theirs_buf = {0};

    git_diff_options diff_options = GIT_DIFF_OPTIONS_INIT;
        
    gk_conflict_diff_summary *summary = gk_conflict_diff_summary_new();
    
    if (git_patch_from_blobs(&ancestor_to_ours_patch, ancestor_blob, entry->path, ours_blob, entry->path, &diff_options) != 0) {
        git_patch_free(ancestor_to_ours_patch);
        gk_session_lg2_failure_ex(session, purpose, GK_ERR, "error generating ancestor<->ours patch");
        return NULL;
    }

    if (git_patch_from_blobs(&ancestor_to_theirs_patch, ancestor_blob, entry->path, theirs_blob, entry->path, &diff_options) != 0) {
        git_patch_free(ancestor_to_theirs_patch);
        git_patch_free(ancestor_to_ours_patch);
        gk_session_lg2_failure_ex(session, purpose, GK_ERR, "error generating ancestor<->theirs patch");
        return NULL;
    }

    if (git_patch_to_buf(&ancestor_to_ours_buf, ancestor_to_ours_patch) != 0) {
        git_patch_free(ancestor_to_theirs_patch);
        git_patch_free(ancestor_to_ours_patch);
        git_buf_dispose(&ancestor_to_ours_buf);
        gk_session_lg2_failure_ex(session, purpose, GK_ERR, "error getting buffer for ancestor<->ours patch");
        return NULL;
    }

    if (git_patch_to_buf(&ancestor_to_theirs_buf, ancestor_to_theirs_patch) != 0) {
        git_patch_free(ancestor_to_theirs_patch);
        git_patch_free(ancestor_to_ours_patch);
        git_buf_dispose(&ancestor_to_ours_buf);
        git_buf_dispose(&ancestor_to_theirs_buf);
        gk_session_lg2_failure_ex(session, purpose, GK_ERR, "error getting buffer for ancestor<->theirs patch");
        return NULL;
    }

    summary->ancestor_to_ours_diff = strdup(ancestor_to_ours_buf.ptr);
    summary->ancestor_to_theirs_diff = strdup(ancestor_to_theirs_buf.ptr);

    gk_session_success(session, purpose);
    return summary;
}

int gk_lg2_index_conflict_get(gk_session *session, const git_index_entry **ancestor_entry, const git_index_entry **ours_entry, const git_index_entry **theirs_entry, git_index *index, const char *path) {
    const char *purpose = "get conflict entry";
    if (gk_session_context_push(session, purpose, &COMP_CONFLICTS, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (git_index_conflict_get(ancestor_entry, ours_entry, theirs_entry, index, path) != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to get conflict entry for path [%s]", path);
    }
    return gk_session_success(session, purpose);
}

int gk_lg2_index_add(gk_session *session, const git_index_entry *entry, const char *path) {
    const char *purpose = "add index entry";
    if (gk_session_context_push(session, purpose, &COMP_CONFLICTS, GK_REPOSITORY_VERIFY_DEFAULT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (git_index_add(session->repository->lg2_resources->merge_index, entry) != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to add index entry at path [%s]", path);
    }
    return gk_session_success(session, purpose);
}

int gk_lg2_oid_from_id(gk_session *session, git_oid *oid, const char *oid_id) {
    const char *purpose = "look up object id from id";
    if (gk_session_context_push(session, purpose, &COMP_CONFLICTS, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (git_oid_fromstr(oid, oid_id) != 0) {
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to find object id corresponding to id [%s]", oid_id);
    }
    return gk_session_success(session, purpose);
}

int gk_lg2_blob_lookup(gk_session *session, git_blob **blob, const git_oid *oid) {
    const char *purpose = "look up blob from object id";
    if (gk_session_context_push(session, purpose, &COMP_CONFLICTS, GK_REPOSITORY_VERIFY_LOCAL_CHECKOUT) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    if (git_blob_lookup(blob, session->repository->lg2_resources->repository, oid) != 0) {
        if (blob != NULL) {
            git_blob_free(*blob);
        }
        return gk_session_lg2_failure_ex(session, purpose, GK_ERR, "failed to look up blob corresponding to id [%s]", git_oid_tostr_s(oid));
    }
    return gk_session_success(session, purpose);
}



