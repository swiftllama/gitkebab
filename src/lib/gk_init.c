
#include "gk_init.h"
#include "stdio.h"
#include "git2.h"
#include "string.h"

#include "gk_results.h"
#include "gk_logging.h"

#define PRIuZ "zu"

static int did_init = 0;
static const char *gitkebab_version = "develop";

void libgit2_log_cb(git_trace_level_t level, const char *msg) {
    // libgit2 logging levels count opposite from us
    // https://libgit2.org/libgit2/#v0.21.2/type/git_trace_level_t
    int safe_level = GIT_TRACE_TRACE - level;
    safe_level = safe_level <= LOG_TRACE ? LOG_TRACE : safe_level;
    log_at_level(GIT_TRACE_TRACE-level, COMP_LIBGIT2, msg); 
}

int gk_did_init() {
    return did_init;
}

void gk_init(const char *log_path, int log_level) {
    if (log_path != NULL) {
        FILE *log_handle = fopen(log_path, "a");
        if (log_handle != NULL) {
            log_add_fp(log_handle, log_level);
        }
        else {
            log_warn(COMP_INIT, "Error opening path [%s] for writing, logging will go to stdout only", log_path);
        }
    }
    
    log_info(COMP_INIT, "Initializing GitKebab v%s", gitkebab_version);
    did_init = 1;
    
    int num_times_init = git_libgit2_init();
    int lg2_ver_major = 0;
    int lg2_ver_minor = 0;
    int lg2_ver_rev = 0;
    git_libgit2_version(&lg2_ver_major, &lg2_ver_minor, &lg2_ver_rev);
    
    log_info(COMP_INIT, "git_libgit2_init() returned %d for libgit2 v%d.%d.%d", num_times_init, lg2_ver_major, lg2_ver_minor, lg2_ver_rev);
    
    if (num_times_init != 1) {
        log_warn(COMP_INIT, "libgit2 initialized %d times (which is more than once)", num_times_init);
    }

    int features = git_libgit2_features();
    if ((features & GIT_FEATURE_SSH) != GIT_FEATURE_SSH) {
        log_warn(COMP_INIT, "libgit2 was not compiled with ssh support!");
    }
    if ((features & GIT_FEATURE_HTTPS) != GIT_FEATURE_HTTPS) {
        log_warn(COMP_INIT, "libgit2 was not compiled with https support!");
    }
    if ((features & GIT_FEATURE_THREADS) != GIT_FEATURE_THREADS) {
        log_warn(COMP_INIT, "libgit2 was not compiled with thread support!");
    }
}


void gk_libgit2_set_log_level(int level) {
    int safe_level = GIT_TRACE_TRACE - level;
    safe_level = safe_level >= 0? safe_level : 0;
    git_trace_set(safe_level, libgit2_log_cb);
}
