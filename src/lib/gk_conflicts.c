
#include "gk_conflicts.h"
#include <stdlib.h>

int gk_conflicts_allocate(gk_session *session, size_t num_conflicts) {
    session->conflict_summary.num_conflicts = num_conflicts;
    session->conflict_summary.conflicts = (gk_merge_conflict_entry **)calloc(sizeof(gk_merge_conflict_entry *), num_conflicts);
    return 0;
}

void gk_conflicts_free(gk_session *session) {
    free(session->conflict_summary.conflicts);
    session->conflict_summary.conflicts = NULL;
}
