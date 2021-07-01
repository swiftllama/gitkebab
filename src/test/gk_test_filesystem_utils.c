
#define _XOPEN_SOURCE 500 
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <ftw.h>
#include <string.h>
#include "gk_logging.h"


int directory_exists(const char* path) {
    DIR* dir = opendir(path);
    if (dir) {
        closedir(dir);
        return 0;
    } else if (errno != ENOENT) {
        log_error(COMP_TEST, "Error %d opening dire '%s' to check if it exists: %s", errno, path, strerror(errno));
    }
    return errno;

}

int file_exists(const char *path) {
    return access(path, F_OK);
}


static int rm_rf_unlink_path(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    int rc = remove(path);
    if (rc != 0) {
        log_error(COMP_TEST, "Error (%d) removing path '%s': %s", errno, path, strerror(errno));
    }
    return rc;
}

int rm_rf(const char *path) {
    return nftw(path, rm_rf_unlink_path, 64, FTW_DEPTH | FTW_PHYS);
}

int create_directory(const char *path) {
    int rc = mkdir(path, 0755);
    if (rc != 0) {
        log_error(COMP_TEST, "Error (%d) creating directory '%s': %s", errno, path, strerror(errno));
    }
    return rc;
}
