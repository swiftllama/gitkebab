
#ifndef __GK_STATUS_H__
#define __GK_STATUS_H__

#include "gk_session.h"


typedef struct {
    unsigned long count_new;
    unsigned long count_modified;
    unsigned long count_deleted;
    unsigned long count_renamed;
    unsigned long count_typechanged;
    unsigned long count_conflicted;
    unsigned long count_total;
    const char *new[];
    const char *modified[];
    const char *deleted[];
    const char *renamed[];
    const char *typechange[];
    const char *conflicted[];
} gk_repository_status_summary_t;

void gk_repository_status_summary(gk_repository_t *repo, gk_repository_status_summary_t *status_summary);

void gk_repository_status_summary_free_members(gk_repository_status_summary_t *status_summary);

#endif // __GK_STATUS_H__
