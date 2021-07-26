
#ifndef __GK_TEST_HELPERS_H__
#define __GK_TEST_HELPERS_H__

#include "gitkebab.h"


void gk_test_session_progress_verbose(gk_session_progress *progress);
void gk_test_session_progress_null(gk_session_progress *progress);

int gk_test_environment_setup(void **state);
int gk_test_environment_teardown(void **state);

void gk_test_copy_source_repo_simplerepo1_dot_git();
void gk_test_copy_simplerepo1_from_simplerepo1_dot_gitbak();
void gk_test_copy_simplerepo1_from_simplerepo1B_mergeconflicts_dot_gitbak();
void gk_test_delete_simplerepo1();
void gk_test_delete_simplerepo1A();
void gk_test_delete_simplerepo1B();
void gk_test_delete_simplerepo1_dot_git();
void gk_test_delete_simplerepo1B_mergeconflicts();

gk_session *gk_test_session_from_local_path(const char *repo_path);
gk_session *gk_test_session_from_clone(const char *remote_repo, const char *local_path);

void gk_test_env_conflicting_repos_a_and_b_with_extended_conflicts(gk_session **session1_ptr, gk_session **session2_ptr, void **state);


#endif // __GK_TEST_HELPERS_H__
