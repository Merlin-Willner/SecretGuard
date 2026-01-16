#ifndef WALK_H
#define WALK_H

#include "cli.h"

/* Directory walking interface for recursive scanning. */

/* Callback for each regular file discovered during walking. */
/* Return non-zero to stop walking early. */
typedef int (*file_visit_callback)(const char *path, void *user_data);

/* Walk the root path and invoke the callback for each regular file. */
/* The implementation uses POSIX dirent/stat APIs under the hood. */
/* Returns 0 on success, or a non-zero value on error. */
int walk_path(const Config *config, file_visit_callback on_file, void *user_data);

#endif /* WALK_H */
