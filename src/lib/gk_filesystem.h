
#ifndef __GK_FILESYSTEM_H__
#define __GK_FILESYSTEM_H__

#include <stdio.h>

#if defined(WIN32)
#define GK_FILESYSTEM_PATH_SEPARATOR "\\"
#else
#define GK_FILESYSTEM_PATH_SEPARATOR "/"
#endif

int gk_concatenate_paths(char *buffer, size_t buffer_length, const char *path1, const char *path2);
int gk_directory_exists(const char *path);
int gk_subdirectory_exists(const char *path, const char* subdirectory);
int gk_directory_is_empty(const char *path);

#endif // __GK_FILESYSTEM_H__
