
#ifndef __GITKEBAB_H__
#define __GITKEBAB_H__

#include "gk_results.h"
#include "gk_session.h"
#include "gk_credentials.h"
#include "gk_status.h"
#include "gk_index.h"
#include "gk_commit.h"
#include "gk_logging.h"

int gk_did_init();
void gk_init();
void gk_libgit2_set_log_level(int level);

#endif // __GITKEBAB_H__
