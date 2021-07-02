
#include "git2.h"
#include "gk_status.h"
#include "gk_logging.h"


gk_result_t *gk_repository_status_summary(gk_repository_t *repo, gk_repository_status_summary_t *status_summary) {
    gk_result_t *result = NULL;
    if (repo == NULL) {
        result = gk_result(-1, "Cannot check status, repo is NULL");
        log_error(COMP_STATUS, gk_result_message(result));
        return result;
    }
    if (status_summary == NULL) {
        result = gk_result(-2, "Cannot check status, status list is NULL");
        log_error(COMP_STATUS, gk_result_message(result));
        return result;
    }
    
    git_status_options status_options = GIT_STATUS_OPTIONS_INIT;
    
    status_options.show  = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    status_options.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
        GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS |
        GIT_STATUS_OPT_RENAMES_INDEX_TO_WORKDIR | 
        GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
        GIT_STATUS_OPT_SORT_CASE_INSENSITIVELY;
    
    
}

void gk_repository_status_summary_free_members(gk_repository_status_summary_t *status_summary) {

}
