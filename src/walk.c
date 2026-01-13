
/* The code in this file should walk through a root path,
go to subfolders to the maxdepth and call a callback for every file it finds  */

#include "walk.h"

#include <dirent.h>  /* POSIX directory traversal (opendir/readdir). */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> /* POSIX stat/lstat for file metadata. */
#include <unistd.h>

/* Join a parent directory and child name into a new path string. */
static char *join_path(const char *parent, const char *child) {
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
        result[position] = '/';
        position++;
    }
    memcpy(result + position, child, child_length + 1);
    return result;
}

/* Recursively walk a path and call a callback for each regular file. */
static int walk_recursive(const Config *config,
                          const char *path,
                          int depth,
                          file_visit_callback on_file,
                          void *user_data) {
    struct stat path_info;
    if (lstat(path, &path_info) != 0) {
        fprintf(stderr, "Failed to stat %s: %s\n", path, strerror(errno));
        return depth == 0 ? -1 : 0;
    }

    if (S_ISLNK(path_info.st_mode)) {
        return 0;
    }

    if (S_ISDIR(path_info.st_mode)) {
        if (config->max_depth >= 0 && depth > config->max_depth) {
            return 0;
        }

        DIR *directory = opendir(path);
        if (!directory) {
            fprintf(stderr, "Failed to open directory %s: %s\n", path, strerror(errno));
            return depth == 0 ? -1 : 0;
        }

        struct dirent *entry = NULL;
        while ((entry = readdir(directory)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char *child = join_path(path, entry->d_name);
            if (!child) {
                closedir(directory);
                return -1;
            }

            if (walk_recursive(config, child, depth + 1, on_file, user_data) != 0) {
                free(child);
                closedir(directory);
                return -1;
            }
            free(child);
        }

        closedir(directory);
        return 0;
    }

    if (!S_ISREG(path_info.st_mode)) {
        return 0;
    }

    if (!on_file) {
        return 0;
    }

    return on_file(path, user_data);
}

/* Walk the root path and scan each file via callback. */
int walk_path(const Config *config, file_visit_callback on_file, void *user_data) {
    if (!config || !config->root_path) {
        return 0;
    }

    return walk_recursive(config, config->root_path, 0, on_file, user_data);
}
