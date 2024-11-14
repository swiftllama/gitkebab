
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "gk_filesystem.h"
#include "gk_logging.h"

int gk_concatenate_paths(char *buffer, size_t buffer_length, const char *path1, const char *path2) {
    if (buffer == NULL) {
        return -1;
    }
    snprintf(buffer, buffer_length, "%s/%s", path1, path2);
    return 0;
}

int gk_subdirectory_exists(const char *path, const char* subdirectory) {
    if ((path == NULL) || (subdirectory == NULL)) return 0;
    int length = strlen(path) + strlen(subdirectory) + 2;
    char *subdirectory_path = (char *)malloc(length);
    if (subdirectory_path == NULL) {
        log_error(COMP_GENERAL, "Failure to allocate memory for gk_subdirectory_exists");
        return 0;
    }
    gk_concatenate_paths(subdirectory_path, length, path, subdirectory);
    int rc = gk_directory_exists(subdirectory_path);
    free(subdirectory_path);
    return rc;
}

int gk_directory_exists(const char *path) {
    DIR* dir = opendir(path);
    if (dir) {
        closedir(dir);
        return 1;
    } else if (errno != ENOENT) {
        log_error(COMP_GENERAL, "Encountered unexpected errno error [%d] while testing if path [%s] exists (expected no error or errno ENOENT=%d)", errno, path, ENOENT);
    }
    return 0;
}

int gk_directory_is_empty(const char *path) {
    DIR* dir = opendir(path);
    struct dirent *entry;
    if (dir) {
        entry = readdir(dir); // read '..'
        entry = readdir(dir); // read '.'
        entry = readdir(dir); // read next entry if it exists
        closedir(dir);
        return entry == NULL;
    } else if (errno != ENOENT) {
        log_error(COMP_GENERAL, "Encountered unexpected errno error [%d] while testing if path [%s] is an empty directory (expected no error or errno ENOENT=%d)", errno, path, ENOENT);
    }
    return 0;
}
