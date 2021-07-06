
#define _XOPEN_SOURCE 500
#include <stdlib.h>
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

int copy_file(const char *source_path, const char *dest_path) {
    if (file_exists(source_path) != 0) {
        log_error(COMP_TEST, "Cannot copy [%s] to [%s], source file does not exist", source_path, dest_path);
        return 1;
    }
    char buffer[4096];
    FILE *read_stream = fopen(source_path, "r");
    FILE *write_stream = fopen(dest_path, "w");
    while (!feof(read_stream)) {
        size_t bytes = fread(buffer, 1, sizeof(buffer), read_stream);
        if (bytes) {
            fwrite(buffer, 1, bytes, write_stream);
        }
    }
    fclose(read_stream);
    fclose(write_stream);

    return 0;
}


int copy_directory(const char *source_path, const char *dest_path) {
    char cp_command[2048];
    snprintf(cp_command, 2048, "cp -PR %s %s", source_path, dest_path);
    return system(cp_command);
}

int mv(const char *source_path, const char *dest_path) {
    char mv_command[2048];
    snprintf(mv_command, 2048, "mv %s %s", source_path, dest_path);
    return system(mv_command);
}
