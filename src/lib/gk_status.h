
#ifndef __GK_STATUS_H__
#define __GK_STATUS_H__

#include "gk_session.h"
#include "gk_results.h"

typedef struct {
    void *status_list;
} gk_repository_status_summary_t;

gk_result_t gk_session_status_summary(gk_repository_t *repo, gk_repository_status_summary_t *status_summary);

void gk_repository_status_summary_free_members(gk_repository_status_summary_t *status_summary);

#endif // __GK_STATUS_H__
