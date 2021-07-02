
#ifndef __GK_STATUS_H__
#define __GK_STATUS_H__

#include "gk_session.h"
#include "gk_results.h"


void gk_session_query_status_summary(gk_session_t *session);

const char *gk_session_status_summary_path_at(gk_session_t *session, size_t index);

void gk_status_summary_reset(gk_status_summary_t *status_summary);


#endif // __GK_STATUS_H__
