
#ifndef __GK_MERGE_H__
#define __GK_MERGE_H__

#include "gk_types.h"

int gk_session_analyze_merge_into_head(gk_session *session, const char* from_ref_name, int *out_analysis);

int gk_session_merge_into_head(gk_session *session);

int gk_session_merge_conflicts_query(gk_session *session, const char *purpose);
#endif // __GK_MERGE_H__
