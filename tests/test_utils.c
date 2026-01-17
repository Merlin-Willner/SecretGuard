#include "test_utils.h"
#include "util.h"
#include "walk.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int test_make_dir(const char *path) {
    if (!path) {
        return -1;
    }
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}

char *test_make_temp_dir(void) {
    if (test_make_dir("build") != 0) {
        return NULL;
    }
    char *template = duplicate_string("build/test_tmp_XXXXXX");
    if (!template) {
        return NULL;
    }
    if (!mkdtemp(template)) {
        free(template);
        return NULL;
    }
    return template;
}

char *test_join_path(const char *parent, const char *child) {
    if (!parent || !child) {
        return NULL;
    }
    size_t parent_length = strlen(parent);
    size_t child_length = strlen(child);
    int needs_separator = (parent_length > 0 && parent[parent_length - 1] != '/');
    size_t total = parent_length + (size_t)needs_separator + child_length + 1;

    char *result = malloc(total);
    if (!result) {
        return NULL;
    }

    memcpy(result, parent, parent_length);
    size_t position = parent_length;
    if (needs_separator) {
        result[position++] = '/';
    }
    memcpy(result + position, child, child_length + 1);
    return result;
}

int test_write_file(const char *path, const char *content) {
    if (!path) {
        return -1;
    }
    FILE *file = fopen(path, "w");
    if (!file) {
        return -1;
    }
    if (content) {
        fputs(content, file);
    }
    fclose(file);
    return 0;
}

int test_write_file_bytes(const char *path, const unsigned char *data, size_t length) {
    if (!path) {
        return -1;
    }
    FILE *file = fopen(path, "wb");
    if (!file) {
        return -1;
    }
    if (data && length > 0) {
        size_t written = fwrite(data, 1, length, file);
        if (written != length) {
            fclose(file);
            return -1;
        }
    }
    fclose(file);
    return 0;
}

static int remove_tree_recursive(const char *path) {
    struct stat info;
    if (lstat(path, &info) != 0) {
        return -1;
    }

    if (S_ISDIR(info.st_mode)) {
        DIR *dir = opendir(path);
        if (!dir) {
            return -1;
        }
        struct dirent *entry = NULL;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            char *child = test_join_path(path, entry->d_name);
            if (!child) {
                closedir(dir);
                return -1;
            }
            (void)remove_tree_recursive(child);
            free(child);
        }
        closedir(dir);
        if (rmdir(path) != 0) {
            return -1;
        }
        return 0;
    }

    if (unlink(path) != 0) {
        return -1;
    }
    return 0;
}

void test_remove_tree(const char *path) {
    if (!path) {
        return;
    }
    (void)remove_tree_recursive(path);
}

int test_redirect_stdin(const char *path, int *saved_fd) {
    if (!path) {
        return -1;
    }
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    int saved = dup(STDIN_FILENO);
    if (saved < 0) {
        close(fd);
        return -1;
    }
    if (dup2(fd, STDIN_FILENO) < 0) {
        close(fd);
        close(saved);
        return -1;
    }
    close(fd);
    if (saved_fd) {
        *saved_fd = saved;
    } else {
        close(saved);
    }
    return 0;
}

void test_restore_stdin(int saved_fd) {
    if (saved_fd < 0) {
        return;
    }
    (void)dup2(saved_fd, STDIN_FILENO);
    close(saved_fd);
}

static int redirect_fd_to_path(int target_fd, const char *path, int *saved_fd) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        return -1;
    }
    int saved = dup(target_fd);
    if (saved < 0) {
        close(fd);
        return -1;
    }
    if (dup2(fd, target_fd) < 0) {
        close(fd);
        close(saved);
        return -1;
    }
    close(fd);
    if (saved_fd) {
        *saved_fd = saved;
    } else {
        close(saved);
    }
    return 0;
}

static void restore_fd(int target_fd, int saved_fd) {
    if (saved_fd < 0) {
        return;
    }
    (void)dup2(saved_fd, target_fd);
    close(saved_fd);
}

int test_redirect_stdout_to_null(int *saved_fd) {
    fflush(stdout);
    return redirect_fd_to_path(STDOUT_FILENO, "/dev/null", saved_fd);
}

int test_redirect_stderr_to_null(int *saved_fd) {
    fflush(stderr);
    return redirect_fd_to_path(STDERR_FILENO, "/dev/null", saved_fd);
}

void test_restore_stdout(int saved_fd) {
    if (saved_fd < 0) {
        return;
    }
    fflush(stdout);
    restore_fd(STDOUT_FILENO, saved_fd);
}

void test_restore_stderr(int saved_fd) {
    if (saved_fd < 0) {
        return;
    }
    fflush(stderr);
    restore_fd(STDERR_FILENO, saved_fd);
}

void init_walk_config(Config *config, const char *root_path, int max_depth) {
    init_config(config);
    config->root_path = duplicate_string(root_path);
    config->max_depth = max_depth;
}

int scan_files_callback(const char *path, void *user_data) {
    if (!path || !user_data) {
        return -1;
    }
    ScanWalkContext *context = (ScanWalkContext *)user_data;
    if (scanner_scan_path(context->scanner, path) != 0) {
        context->error = 1;
        return -1;
    }
    return 0;
}

size_t count_findings_with_depth(const char *root_path, int max_depth) {
    RulesEngine rules;
    ScannerContext scanner;
    if (rules_init(&rules) != 0) {
        return 0;
    }
    scanner_init(&scanner, &rules);

    Config config;
    init_walk_config(&config, root_path, max_depth);

    ScanWalkContext context = {&scanner, 0};
    if (walk_path(&config, scan_files_callback, &context) != 0 || context.error != 0) {
        free_config(&config);
        scanner_destroy(&scanner);
        rules_destroy(&rules);
        return 0;
    }

    free_config(&config);
    size_t findings = scanner.finding_count;
    scanner_destroy(&scanner);
    rules_destroy(&rules);
    return findings;
}
