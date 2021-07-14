
#ifndef __GK_CONFLICS_H__
#define __GK_CONFLICTS_H__

#include "gk_types.h"

void gk_conflicts_free(gk_session *session);
int gk_conflicts_allocate(gk_session *session, size_t num_conflicts);

#endif // __GK_CONFLICTS_H__
