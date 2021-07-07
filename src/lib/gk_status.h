
#ifndef __GK_STATUS_H__
#define __GK_STATUS_H__

#include "gk_session.h"
#include "gk_results.h"


int gk_session_query_status_summary(gk_session *session);

const char *gk_session_status_summary_path_at(gk_session *session, size_t index);

int gk_session_status_summary_status_at(gk_session *session, size_t index);

void gk_status_summary_reset(gk_status_summary *status_summary);

size_t gk_session_status_summary_entrycount(gk_session *session);


#endif // __GK_STATUS_H__
