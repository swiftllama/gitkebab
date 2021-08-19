
#ifndef __GK_MERGE_H__
#define __GK_MERGE_H__

#include "gk_types.h"

int gk_analyze_merge_into_head(gk_session *session, const char* from_ref_name, int *out_analysis);
int gk_merge_conflicts_query(gk_session *session);
int gk_merge_into_head(gk_session *session);
int gk_merge_into_head_finalize(gk_session *session);
int gk_merge_abort(gk_session *session);

const char *gk_merge_conflict_entry_ancestor_oid_id(gk_merge_conflict_entry *entry);
const char *gk_merge_conflict_entry_ours_oid_id(gk_merge_conflict_entry *entry);
const char *gk_merge_conflict_entry_theirs_oid_id(gk_merge_conflict_entry *entry);
const char *gk_merge_conflict_summary_fetch_head_oid_id(gk_repository *repository);
const char *gk_merge_conflict_summary_repository_head_oid_id(gk_repository *repository);

#endif // __GK_MERGE_H__
