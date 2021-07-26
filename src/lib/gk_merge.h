
#ifndef __GK_MERGE_H__
#define __GK_MERGE_H__

#include "gk_types.h"

int gk_analyze_merge_into_head(gk_session *session, const char* from_ref_name, int *out_analysis);
int gk_merge_conflicts_query(gk_session *session);
static int merge_fast_forward(gk_session *session, const char *from_ref_name);
int gk_merge_into_head(gk_session *session);
int gk_merge_into_head_finalize(gk_session *session);
int gk_merge_abort(gk_session *session);


#endif // __GK_MERGE_H__
