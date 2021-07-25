
#ifndef __GK_STATUS_H__
#define __GK_STATUS_H__

#include "gk_repository.h"
#include "gk_results.h"


int gk_repository_status_summary_query(gk_repository *repository);
void gk_repository_status_summary_close(gk_repository *repository);

const char *gk_repository_status_summary_path_at(gk_repository *repository, size_t index);

int gk_repository_status_summary_status_at(gk_repository *repository, size_t index);

void gk_status_summary_reset(gk_status_summary *status_summary);

size_t gk_repository_status_summary_entrycount(gk_repository *repository);


#endif // __GK_STATUS_H__
