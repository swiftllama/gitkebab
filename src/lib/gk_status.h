
#ifndef __GK_STATUS_H__
#define __GK_STATUS_H__

#include "gk_repository.h"
#include "gk_results.h"

void gk_status_summary_reset(gk_status_summary *status_summary);
void gk_status_summary_close(gk_session *session);
int gk_status_summary_query(gk_session *session);
int gk_status_summary_status_at(gk_session *session, size_t index);
const char *gk_status_summary_path_at(gk_session *session, size_t index);
size_t gk_status_summary_entrycount(gk_session *session);

#endif // __GK_STATUS_H__
