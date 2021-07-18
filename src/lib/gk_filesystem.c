
#include "gk_filesystem.h"

int gk_concatenate_paths(char *buffer, size_t buffer_length, const char *path1, const char *path2) {
    if (buffer == NULL) {
        return -1;
    }
    snprintf(buffer, buffer_length, "%s/%s", path1, path2);
    return 0;
}
