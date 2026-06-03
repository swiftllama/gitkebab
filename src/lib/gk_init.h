
#ifndef __GK_INIT_H__
#define __GK_INIT_H__

int gk_did_init();
void gk_init(const char *log_path, int log_level);
void gk_libgit2_set_log_level(int level);

// Overrides the HOME environment variable; see gk_init.c for the
// rationale. Returns 0 on success, -1 on setenv failure.
int gk_set_home(const char *path);

#endif // __GK_INIT_H__
