
#ifndef __GK_TEST_FILESYSTEM_UTILS__
#define __GK_TEST_FILESYSTEM_UTILS__

int directory_exists(const char* path);
int file_exists(const char *path);
int rm_rf(const char* path);
int create_directory(const char *path);

#endif // _GK_TEST_FILESYSTEM_UTILS
