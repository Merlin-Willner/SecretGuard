#ifndef WALK_H
#define WALK_H

#include "cli.h"

// Callback for each regular file. Return non-zero to stop early.
typedef int (*file_visit_callback)(const char *path, void *user_data);

// Walk the root path and call the callback for each regular file.
// Returns 0 on success, non-zero on error.
int walk_path(const Config *config, file_visit_callback on_file, void *user_data);

#endif /* WALK_H */
