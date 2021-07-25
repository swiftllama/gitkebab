
#ifndef __GK_MERGE_H__
#define __GK_MERGE_H__

#include "gk_types.h"

int gk_repository_analyze_merge_into_head(gk_repository *repository, const char* from_ref_name, int *out_analysis);

int gk_repository_merge_into_head(gk_repository *repository);

int gk_repository_merge_conflicts_query(gk_repository *repository, const char *purpose);

int gk_repository_merge_into_head_finalize(gk_repository *repository);

int gk_repository_merge_abort(gk_repository *repository);

#endif // __GK_MERGE_H__
