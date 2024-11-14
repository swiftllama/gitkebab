
#ifndef __GK_TEST_FILESYSTEM_UTILS__
#define __GK_TEST_FILESYSTEM_UTILS__

int directory_exists(const char* path);
int file_exists(const char *path);
int rm_file(const char *file_path);
int rm_rf(const char* path);
int create_directory(const char *path);
int copy_file(const char *source_path, const char *dest_path);
int copy_directory(const char *source_path, const char *dest_path);
int mv(const char *source_path, const char *dest_path);
int diff(const char *source_path, const char *dest_path);
int make_executable(const char *path);
void prepare_error_listing(const char *error_listing_name);
void append_to_error_listing(const char *error_listing_name, const char *scenario, const char *returned_error_code, const char *context_stack_trace);

#endif // _GK_TEST_FILESYSTEM_UTILS
