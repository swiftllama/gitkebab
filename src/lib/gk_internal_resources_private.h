
#ifndef _GK_INTERNAL_RESOURCES_PRIVATE_H__
#define _GK_INTERNAL_RESOURCES_PRIVATE_H__

#include "git2.h"

typedef struct {
    git_reference *repository_head_ref;
    git_object *repository_head_object;
    const git_oid *repository_head_oid;
    char repository_head_oid_id[41];
    
    git_reference *fetch_head_ref;
    git_object *fetch_head_object;
    git_annotated_commit *annotated_fetch_head_commit;
    const git_oid *fetch_head_oid;
    char fetch_head_oid_id[41];
    

    git_index *index;
    git_tree *tree;

    git_signature *signature;

    git_commit **merge_parents;
    
} gk_internal_resources;


void gk_internal_resources_free(gk_internal_resources *resources);
gk_internal_resources *gk_internal_resources_new();

void gk_internal_resources_free_references(gk_internal_resources *resources);

int gk_internal_resources_load_references(gk_session *session, gk_internal_resources *resources, const char *from_ref_name, const char *purpose);

#endif // _GK_INTERNAL_RESOURCES_PRIVATE_H__
