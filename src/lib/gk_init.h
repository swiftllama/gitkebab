
#ifndef __GK_INIT_H__
#define __GK_INIT_H__

int gk_did_init();
void gk_init(const char *log_path, int log_level);
void gk_libgit2_set_log_level(int level);

#endif // __GK_INIT_H__
