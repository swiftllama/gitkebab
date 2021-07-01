
#ifndef __GK_STATUS_H__
#define __GK_STATUS_H__

void gk_repository_status_summary(..);

typedef struct {
    const char *new[];
    const char *modified[];
    const char *deleted[];
    const char *renamed[];
    const char *typechange[];
    const char *conflicted[];
} gk_repository_status_summary_t;
#endif // __GK_STATUS_H__
