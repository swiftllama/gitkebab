
#ifndef __GITKEBAB_H__
#define __GITKEBAB_H__

#include "results.h"
#include "ssh_keys.h"
#include "gk_session.h"

void gk_init();
void gk_libgit2_set_log_level(int level);

extern log_Component COMP_INIT;
#endif // __GITKEBAB_H__
