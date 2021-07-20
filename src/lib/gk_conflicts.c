
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "git2.h"
#include "git2/sys/hashsig.h"

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



int gk_conflict_resolve_accept_existing(gk_session *session, const char *path, gk_conflict_resolution accept) {
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
    else if (accept == GK_CONFLICT_RESOLUTION_ANCESTOR) {
        purpose = "resolve conflict (accept ancestor)";
    }
    else {
        return gk_session_failure(session, &COMP_CONFLICTS, -6, "Canont resolve conflict for path '%s', unknown resolution type '%d' (expected ours '%d' or theirs '%d')", accept, GK_CONFLICT_RESOLUTION_OURS, GK_CONFLICT_RESOLUTION_THEIRS);
    }

    // NOTE: we have to get the normal entry from the current index,
    // as the merge index only has it as a staged ancestor/ours/theirs
    // entry. This feels a bit sketchy
    if (gk_lg2_index_load(session, purpose) != GK_SUCCESS) {
        gk_lg2_index_free(session);
        return GK_FAILURE;
    }
    
    const git_index_entry *conflicted_entry = git_index_get_bypath(session->lg2_resources->index, path, GIT_INDEX_STAGE_NORMAL);
    if (conflicted_entry == NULL) {
        return gk_session_failure(session, &COMP_MERGE, -6, "Cannot %s, file not found in index at path %s", purpose, path);
    }
    gk_lg2_index_free(session);

    const git_index_entry *ancestor_entry = NULL;
    const git_index_entry *ours_entry = NULL;
    const git_index_entry *theirs_entry = NULL;

    if (gk_lg2_index_conflict_get(session, &ancestor_entry, &ours_entry, &theirs_entry, session->lg2_resources->merge_index, path, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    git_index_entry clean_entry = *conflicted_entry;

    if (accept == GK_CONFLICT_RESOLUTION_OURS) {
        if (ours_entry == NULL) {
            return gk_session_failure(session, &COMP_CONFLICTS, -11, "Cannot %s, conflict has no ours entry");
        }
        clean_entry.id = ours_entry->id;
    }
    else if (accept == GK_CONFLICT_RESOLUTION_THEIRS) {
        if (theirs_entry == NULL) {
            return gk_session_failure(session, &COMP_CONFLICTS, -11, "Cannot %s, conflict has no theirs entry");
        }
        clean_entry.id = theirs_entry->id;
    }
    else if (accept == GK_CONFLICT_RESOLUTION_ANCESTOR) {
        if (ancestor_entry == NULL) {
            return gk_session_failure(session, &COMP_CONFLICTS, -11, "Cannot %s, conflict has no ancestor entry");
        }
        clean_entry.id = ancestor_entry->id;
    }

    if (git_index_remove_bypath(session->lg2_resources->merge_index, path) != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CONFLICTS, -1, "Cannot %s, error removing file [%s] from index so as to clear its conflict sstatus (%d): %s", purpose, path, err->klass, err->message);
    }

    if (git_index_add(session->lg2_resources->merge_index, &clean_entry) != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CONFLICTS, -1, "Cannot %s, error adding file [%s] to index (%d): %s", purpose, path, err->klass, err->message);
    }

    return GK_SUCCESS;
}

int gk_blob_contents(gk_session *session, void **blob_data, uint64_t *blob_data_length, const char *oid_id, const char *purpose) {
    if (gk_session_verify(session, &COMP_CONFLICTS, GK_SESSION_VERIFY_LOCAL_CHECKOUT, "get blob contents") != GK_SUCCESS) {
        return GK_FAILURE;
    }
    else if (blob_data == NULL) {
        return gk_session_failure(session, &COMP_CONFLICTS, -4, "Cannot %s, error getting blob content with NULL data pointer", purpose);
    }
    else if (blob_data_length == NULL) {
        return gk_session_failure(session, &COMP_CONFLICTS, -4, "Cannot %s, error getting blob content with NULL data length pointer", purpose);
    }
    else if (oid_id == NULL) {
        return gk_session_failure(session, &COMP_CONFLICTS, -4, "Cannot %s, error getting blob content with NULL object id", purpose);
    }
    else if (oid_id[0] == '\0') {
        return gk_session_failure(session, &COMP_CONFLICTS, -4, "Cannot %s, error getting blob content with empty object id", purpose);
    }
    
    git_oid blob_oid;
    if (gk_lg2_oid_from_id(session, &blob_oid, oid_id, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    git_blob *blob = NULL;
    if (gk_lg2_blob_lookup(session, &blob, &blob_oid, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }

    *blob_data_length = git_blob_rawsize(blob);
    *blob_data = malloc(*blob_data_length);
    if (blob_data == NULL) {
        return gk_session_failure(session, &COMP_CONFLICTS, -7, "Cannot %s, error allocating memory for blob data");
    }
    memcpy(*blob_data, git_blob_rawcontent(blob), *blob_data_length);

    git_blob_free(blob);
    return GK_SUCCESS;
}

int gk_blob_write_contents(gk_session *session, const char *oid_id, const char *path, const char* purpose) {
    if (gk_session_verify(session, &COMP_CONFLICTS, GK_SESSION_VERIFY_LOCAL_CHECKOUT, "write blob") != GK_SUCCESS) {
        return GK_FAILURE;
    }

    uint64_t blob_size = 0;
    void *blob_data = NULL;
    if (gk_blob_contents(session, &blob_data, &blob_size, oid_id, purpose) != GK_SUCCESS) {
        free(blob_data);
        return GK_FAILURE;
    }
    
    FILE *fptr = fopen(path, "w");
    if (fptr == NULL) {
        return gk_session_failure(session, &COMP_CONFLICTS, -10, "Cannot %s, error opening file [%s] for writing: error %d occurred", purpose, path, errno);
    }

    int rc = fwrite(blob_data, 1, blob_size, fptr);
    fclose(fptr);
    if (rc < 0) {
        return gk_session_failure(session, &COMP_CONFLICTS, -10, "Cannot %s, error writing to file [%s]: error %d occurred", purpose, path, rc);
    }

    log_info(COMP_CONFLICTS, "Wrote blob with object id [%s] to [%s]", oid_id, path);
    
    free(blob_data);
    blob_data = NULL;
    
    return GK_SUCCESS;
}


static int gk_conflict_resolve_accept_delete(gk_session *session, const char *path, const char *purpose) {
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

    log_info(COMP_CONFLICTS, "resolved conflict for path [%s] (%s)", path, purpose);
    return GK_SUCCESS ;   
}

int gk_conflict_resolve_accept_remote_delete(gk_session *session, const char *path) {
    const char *purpose = "resolve conflict by accepting remote delete";
    return gk_conflict_resolve_accept_delete(session, path, purpose);
}

int gk_conflict_resolve_accept_local_delete(gk_session *session, const char *path) {
    const char *purpose = "resolve conflict by accepting local delete";
    return gk_conflict_resolve_accept_delete(session, path, purpose);
}

const char *gk_conflict_merged_buffer_with_conflict_markers(gk_session *session, const char *ancestor_oid_id, const char *ours_oid_id, const char *theirs_oid_id, const char *path) {
    const char *purpose = "generate merged buffer";
    if (gk_session_verify(session, &COMP_CONFLICTS, GK_SESSION_VERIFY_LOCAL_CHECKOUT | GK_SESSION_VERIFY_MERGE_IN_PROGRESS, purpose) != GK_SUCCESS) {
        return NULL;
    }
    else if (ancestor_oid_id == NULL) {
        gk_session_failure(session, &COMP_CONFLICTS, -5, "Cannot %s with NULL ancestor object id", purpose);
        return NULL;
    }
    else if (ours_oid_id == NULL) {
        gk_session_failure(session, &COMP_CONFLICTS, -5, "Cannot %s with NULL ours object id", purpose);
        return NULL;
    }
    else if (theirs_oid_id == NULL) {
        gk_session_failure(session, &COMP_CONFLICTS, -5, "Cannot %s with NULL theirs object id", purpose);
        return NULL;
    }

    git_merge_file_input ancestor_input;
    git_merge_file_input ours_input;
    git_merge_file_input theirs_input;

    git_merge_file_input_init(&ancestor_input, GIT_MERGE_FILE_INPUT_VERSION);
    git_merge_file_input_init(&ours_input, GIT_MERGE_FILE_INPUT_VERSION);
    git_merge_file_input_init(&theirs_input, GIT_MERGE_FILE_INPUT_VERSION);

    ancestor_input.path = path;
    ours_input.path = path;
    theirs_input.path = path;

    if (gk_blob_contents(session, (void **)&ancestor_input.ptr, &ancestor_input.size, ancestor_oid_id, purpose) != GK_SUCCESS) {
        return NULL;
    }
    if (gk_blob_contents(session, (void **)&ours_input.ptr, &ours_input.size, ours_oid_id, purpose) != GK_SUCCESS) {
        return NULL;
    }
    if (gk_blob_contents(session, (void **)&theirs_input.ptr, &theirs_input.size, theirs_oid_id, purpose) != GK_SUCCESS) {
        return NULL;
    }

    git_merge_file_options merge_options;
    git_merge_file_options_init(&merge_options, GIT_MERGE_OPTIONS_VERSION);
    merge_options.flags = GIT_MERGE_FILE_STYLE_DIFF3 | GIT_MERGE_FILE_DIFF_MINIMAL;
    
    git_merge_file_result merged_file;
    if (git_merge_file(&merged_file, &ancestor_input, &ours_input, &theirs_input, &merge_options) != 0) {
        const git_error *err = git_error_last();
        gk_session_failure(session, &COMP_CONFLICTS, -7, "Cannot %s, error while merging file (%d): %s", err->klass, err->message);
        return NULL;
    }

    char *data = (char *)malloc(merged_file.len);
    memcpy(data, merged_file.ptr, merged_file.len);
    git_merge_file_result_free(&merged_file);

    return data;
}

void gk_conflict_merged_buffer_free(const char *buffer) {
    free((void *)buffer);
}


// NOTE: this adds a blob to the current repository, but resolves the
// conflict in the MERGE index. If this merge is abandoned it could
// potentially leave this dangling blob which would eventually simply
// get recycled
int gk_conflict_resolve_from_buffer(gk_session *session, const char *path, void *data, u_int64_t data_length) {
    const char *purpose = "resolve conflict by accepting data from buffer";
    if (gk_session_verify(session, &COMP_CONFLICTS, GK_SESSION_VERIFY_LOCAL_CHECKOUT | GK_SESSION_VERIFY_MERGE_IN_PROGRESS, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    else if (path == NULL) {
        return gk_session_failure(session, &COMP_CONFLICTS, -9, "Cannot %s, path is NULL", purpose);
    }
    else if (data == NULL) {
        return gk_session_failure(session, &COMP_CONFLICTS, -9, "Cannot %s, buffer is NULL", purpose);
    }

    // Note: cannot add buffer directly to merge_index because it's
    // not backed by the repository, it only exists in memory. So
    // we add the blob to the real index
    git_oid blob_oid;
    if (git_blob_create_from_buffer(&blob_oid, session->lg2_resources->repository, data, data_length) != 0) {
        return gk_session_failure(session, &COMP_CONFLICTS, -6, "Cannot %s, error adding buffer data to blob", purpose, path);
    }

    log_info(COMP_CONFLICTS, "Created blog [%s] to resolve conflict at path [%s]", git_oid_tostr_s(&blob_oid), path);


    // NOTE: we have to get the normal entry from the current index,
    // as the merge index only has it as a staged ancestor/ours/theirs
    // entry. This feels a bit sketchy
    if (gk_lg2_index_load(session, purpose) != GK_SUCCESS) {
        gk_lg2_index_free(session);
        return GK_FAILURE;
    }

    const git_index_entry *conflicted_entry = git_index_get_bypath(session->lg2_resources->index, path, GIT_INDEX_STAGE_NORMAL);
    if (conflicted_entry == NULL) {
        return gk_session_failure(session, &COMP_CONFLICTS, -6, "Cannot %s, ancestor file not found in index at path [%s]", purpose, path);
    }
    gk_lg2_index_free(session);

    git_index_entry clean_entry = *conflicted_entry;
    clean_entry.id = blob_oid;
    
    if (git_index_remove_bypath(session->lg2_resources->merge_index, path) != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CONFLICTS, -1, "Cannot %s, error removing file [%s] from index so as to clear its conflict sstatus (%d): %s", purpose, path, err->klass, err->message);
    }

    if (git_index_add(session->lg2_resources->merge_index, &clean_entry) != 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CONFLICTS, -1, "Cannot %s, error adding file [%s] to index (%d): %s", purpose, path, err->klass, err->message);
    }

    return GK_SUCCESS;
}

int gk_compare_blobs(gk_session *session, int *similarity, const char *blob1_oid_id, const char *blob2_oid_id, const char *purpose) {
    if (gk_session_verify(session, &COMP_CONFLICTS, GK_SESSION_VERIFY_LOCAL_CHECKOUT, purpose) != GK_SUCCESS) {
        return GK_FAILURE;
    }
    else if (blob1_oid_id == NULL) {
        return gk_session_failure(session, &COMP_CONFLICTS, -12, "Cannot %s, blob1 id is NULL");
    }
    else if (blob2_oid_id == NULL) {
        return gk_session_failure(session, &COMP_CONFLICTS, -12, "Cannot %s, blob2 id is NULL");
    }

    log_info(COMP_CONFLICTS, "Calculating similarity between blob1 [%s] and blob2 [%s]", blob1_oid_id, blob2_oid_id);
    
    uint64_t blob_size1 = 0;
    void *blob_data1 = NULL;
    if (gk_blob_contents(session, &blob_data1, &blob_size1, blob1_oid_id, purpose) != GK_SUCCESS) {
        free(blob_data1);
        return GK_FAILURE;
    }

    uint64_t blob_size2 = 0;
    void *blob_data2 = NULL;
    if (gk_blob_contents(session, &blob_data2, &blob_size2, blob2_oid_id, purpose) != GK_SUCCESS) {
        free(blob_data2);
        return GK_FAILURE;
    }

    git_hashsig *blob1_hashsig;
    git_hashsig *blob2_hashsig;
    
    if (git_hashsig_create(&blob1_hashsig, blob_data1, blob_size1, GIT_HASHSIG_ALLOW_SMALL_FILES) != 0) {
        free(blob_data1);
        free(blob_data2);
        git_hashsig_free(blob1_hashsig);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CONFLICTS, -8, "Cannot %s, error creating hash signature for blob [%s] (%d): %s", purpose, blob1_oid_id, err->klass, err->message);
    }

    if (git_hashsig_create(&blob2_hashsig, blob_data2, blob_size2, GIT_HASHSIG_ALLOW_SMALL_FILES) != 0) {
        free(blob_data1);
        free(blob_data2);
        git_hashsig_free(blob1_hashsig);
        git_hashsig_free(blob2_hashsig);
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CONFLICTS, -8, "Cannot %s, error creating hash signature for blob [%s] (%d): %s", purpose, blob2_oid_id, err->klass, err->message);
    }

    int blob_similarity = git_hashsig_compare(blob1_hashsig, blob2_hashsig);

    free(blob_data1);
    free(blob_data2);
    git_hashsig_free(blob1_hashsig);
    git_hashsig_free(blob2_hashsig);
        
    if (blob_similarity < 0) {
        const git_error *err = git_error_last();
        return gk_session_failure(session, &COMP_CONFLICTS, -8, "Cannot %s, error calculating similarity between blobs (%d): %s", purpose, err->klass, err->message);
    }

    *similarity = blob_similarity;

    return GK_SUCCESS;
}
