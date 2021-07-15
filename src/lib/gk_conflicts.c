
#include "gk_conflicts.h"
#include <stdlib.h>

int gk_conflicts_allocate(gk_session *session, size_t num_conflicts) {
    session->conflict_summary.num_conflicts = num_conflicts;
    session->conflict_summary.conflicts = (gk_merge_conflict_entry **)calloc(sizeof(gk_merge_conflict_entry *), num_conflicts);
    return 0;
}

void gk_conflicts_free(gk_session *session) {
    for (size_t i = 0; i < session->conflict_summary.num_conflicts; i += 1) {
        free(session->conflict_summary.conflicts[i]);
        session->conflict_summary.conflicts[i] = NULL;
    }
    free(session->conflict_summary.conflicts);
    // TODO free indifivual conflicts?
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
