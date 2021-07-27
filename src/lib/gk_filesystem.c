
#include <dirent.h>
#include <errno.h>

#include "gk_filesystem.h"
#include "gk_logging.h"

int gk_concatenate_paths(char *buffer, size_t buffer_length, const char *path1, const char *path2) {
    if (buffer == NULL) {
        return -1;
    }
    snprintf(buffer, buffer_length, "%s/%s", path1, path2);
    return 0;
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
