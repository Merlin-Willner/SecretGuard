
/* The code in this file should walk through a root path,
go to subfolders to the maxdepth and call a callback for every file it finds  */

#include "walk.h"

int walk_path(const Config *config, file_visit_callback on_file, void *user_data) {
    (void)on_file;
    (void)user_data;

    if (!config || !config->root_path) {
        return 0;
    }

    /* TODO: implement directory traversal */
    return 0;
}