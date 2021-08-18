
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

extern const int GK_FILE_STATUS_CURRENT;
extern const int GK_FILE_STATUS_INDEX_NEW;
extern const int GK_FILE_STATUS_INDEX_MODIFIED;
extern const int GK_FILE_STATUS_INDEX_DELETED;
extern const int GK_FILE_STATUS_INDEX_RENAMED;
extern const int GK_FILE_STATUS_INDEX_TYPECHANGE;
extern const int GK_FILE_STATUS_WT_NEW;
extern const int GK_FILE_STATUS_WT_MODIFIED;
extern const int GK_FILE_STATUS_WT_DELETED;
extern const int GK_FILE_STATUS_WT_TYPECHANGE;
extern const int GK_FILE_STATUS_WT_RENAMED;
extern const int GK_FILE_STATUS_WT_UNREADABLE;
extern const int GK_FILE_STATUS_IGNORED;
extern const int GK_FILE_STATUS_CONFLICTED;

#endif // __GK_STATUS_H__
