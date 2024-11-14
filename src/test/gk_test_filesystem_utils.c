
#if defined(__APPLE__)
// NOTE: macos wants stdio.h before _XOPEN_SOURCE
#include <stdio.h>
#define _XOPEN_SOURCE 500
#else
#define _XOPEN_SOURCE 500
#include <stdio.h>
#endif
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <ftw.h>
#include <string.h>
#include "gk_logging.h"
#if defined(_WIN32) || defined(__WIN32__)
//NOTE: mingw include for _mkdir
#include <direct.h>
#endif
                   
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


int parent_directory_exists(const char* path) {
    char command[1024];
    snprintf(command, 1023, "stat `dirname %s`", path);
    return system(command);
}

#if !defined(_WIN32) && !defined(__WIN32__)
static int rm_rf_unlink_path(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    (void) sb;
    (void) typeflag;
    (void) ftwbuf;

    int rc = remove(path);
    if (rc != 0) {
        log_error(COMP_TEST, "Error (%d) removing path '%s': %s", errno, path, strerror(errno));
    }
    return rc;
}
#endif

int rm_rf(const char *path) {
#if defined(_WIN32) || defined(__WIN32__)
    char command[1024];
    snprintf(command, 1023, "rmdir /s/q \"%s\"", path);
    int rc = system(command);
    return rc;
#else
    return nftw(path, rm_rf_unlink_path, 64, FTW_DEPTH | FTW_PHYS);
#endif
}

int rm_file(const char *file_path) {
#if defined(_WIN32) || defined(__WIN32__)
    char command[1024];
    snprintf(command, 1023, "del \"%s\"", file_path);
    for (int i = 0; i < 1024; i += 1) {
        if (command[i] == '/') command[i] = '\\';
    }
    printf("executing rm-file command [%s]\n", command);
    int rc = system(command);
    return rc;
#else
    return rm_rf(file_path);
#endif
}

int create_directory(const char *path) {
#if defined(_WIN32) || defined(__WIN32__)
    return _mkdir(path);
#else
    return mkdir(path, 0755);
#endif
}

int copy_file(const char *source_path, const char *dest_path) {
#if defined(_WIN32) || defined(__WIN32__)
    char cp_command[2048];
    snprintf(cp_command, 2048, "copy %s %s", source_path, dest_path);
    for (int i = 0; i < 2048; i += 1) {
        if (cp_command[i] == '/') cp_command[i] = '\\';
    }
    printf("executing copy-file command [%s]\n", cp_command);
    int rc = system(cp_command);
    return rc;
#else
    if (file_exists(source_path) != 0) {
        log_error(COMP_TEST, "Cannot copy [%s] to [%s], source file does not exist", source_path, dest_path);
        return 1;
    }
    if (parent_directory_exists(dest_path) != 0) {
        log_error(COMP_TEST, "Cannot copy [%s] to [%s], dest parent directory does not exist", source_path, dest_path);
        return 2;
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
#endif
}


int copy_directory(const char *source_path, const char *dest_path) {
    char cp_command[2048];
#if defined(_WIN32) || defined(__WIN32__)
    snprintf(cp_command, 2048, "robocopy /np /ndl /nfl /nc /ns /njh /njs /E %s %s", source_path, dest_path);
#else
    snprintf(cp_command, 2048, "cp -PR %s %s", source_path, dest_path);
#endif
    return system(cp_command);
}

int diff(const char *source_path, const char *dest_path) {
    char command[2048];
#if defined(_WIN32) || defined(__WIN32__)
    snprintf(command, 2048, "fc %s %s", source_path, dest_path);
    for (int i = 0; i < 2048; i += 1) {
        if (command[i] == '/') command[i] = '\\';
    }
#else
    snprintf(command, 2048, "diff %s %s", source_path, dest_path);
#endif
    return system(command);
}

int mv(const char *source_path, const char *dest_path) {
#if defined(_WIN32) || defined(__WIN32__)
    return rename(source_path, dest_path);
#else
    char mv_command[2048];
    snprintf(mv_command, 2048, "mv %s %s", source_path, dest_path);
    return system(mv_command);
#endif
}

int make_executable(const char *path) {
    char command[2048];
#if defined(_WIN32) || defined(__WIN32__)
    (void)path;
    printf("MAKING A FILE EXECUTABLE NOT YET IMPLEMENTED ON WINDOWS\n");
    return -1;
#else
    snprintf(command, 2048, "chmod +x %s", path);
#endif
    return system(command);
}

void prepare_error_listing(const char *error_listing_name) {
#if defined(_WIN32) || defined(__WIN32__)
    (void) error_listing_name; // unused
    printf("Error listing not implemented on windows, see gk_test_filesystem_utils.c\n");
#else
    char catalog_path[256];
    snprintf(catalog_path, 256, "test-staging/error-catalog-%s", error_listing_name);
    rm_rf(catalog_path);
    char cp_command[2048];
    snprintf(cp_command, 2048, "touch %s", catalog_path);
    int rc = system(cp_command);
    (void) rc;
#endif
}

void append_to_error_listing(const char *error_listing_name, const char *scenario, const char *returned_error_code, const char *context_stack_trace) {
#if defined(_WIN32) || defined(__WIN32__)
    (void) error_listing_name; //unused
    (void) scenario;
    (void) returned_error_code;
    (void) context_stack_trace;
    printf("append to error listing  not implemented on windows, see gk_test_filesystem_utils.c\n");
#else
    char catalog_path[256];
    snprintf(catalog_path, 256, "test-staging/error-catalog-%s", error_listing_name);   
    FILE *write_stream = fopen(catalog_path, "a");
    const char *header = "\n----------------\n\nSCENARIO: ";
    const char *returned_label = "\n\nERROR CODE: ";
    const char *stack_trace_label = "\n\nCONTEXT STACK TRACE: \n";
    fwrite(header, 1, strlen(header), write_stream);
    fwrite(scenario, 1, strlen(scenario), write_stream);
    fwrite(returned_label, 1, strlen(returned_label), write_stream);
    fwrite(returned_error_code, 1, strlen(returned_error_code), write_stream);
    fwrite(stack_trace_label, 1, strlen(stack_trace_label), write_stream);
    fwrite(context_stack_trace, 1, strlen(context_stack_trace), write_stream);
    fwrite("\n", 1, 1, write_stream);
    fclose(write_stream);
#endif
}
