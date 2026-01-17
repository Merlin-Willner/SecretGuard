#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stddef.h>

#include "config.h"
#include "scanner.h"

char *test_make_temp_dir(void);
char *test_join_path(const char *parent, const char *child);
int test_make_dir(const char *path);
int test_write_file(const char *path, const char *content);
int test_write_file_bytes(const char *path, const unsigned char *data, size_t length);
void test_remove_tree(const char *path);
int test_redirect_stdin(const char *path, int *saved_fd);
void test_restore_stdin(int saved_fd);
int test_redirect_stdout_to_null(int *saved_fd);
int test_redirect_stderr_to_null(int *saved_fd);
void test_restore_stdout(int saved_fd);
void test_restore_stderr(int saved_fd);

void init_walk_config(Config *config, const char *root_path, int max_depth);

typedef struct {
    ScannerContext *scanner;
    int error;
} ScanWalkContext;

int scan_files_callback(const char *path, void *user_data);
size_t count_findings_with_depth(const char *root_path, int max_depth);

#endif /* TEST_UTILS_H */
