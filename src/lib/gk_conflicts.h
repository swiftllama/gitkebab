
#ifndef __GK_CONFLICS_H__
#define __GK_CONFLICTS_H__

#include "gk_types.h"

void gk_conflicts_free(gk_session *session);
int gk_conflicts_allocate(gk_session *session, size_t num_conflicts);

void gk_conflict_diff_summary_free(gk_conflict_diff_summary *summary);
gk_conflict_diff_summary *gk_conflict_diff_summary_new();

void free_void_node_chain(void_linked_node *chain, int free_data);
void_linked_node *void_linked_node_new();

#endif // __GK_CONFLICTS_H__
