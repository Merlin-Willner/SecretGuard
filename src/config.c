#include "config.h"

#include <stdlib.h>

// Set default values.
void init_config(Config *config) {
    if (!config) {
        return;
    }
    config->root_path = NULL;
    config->max_depth = DEFAULT_MAX_DEPTH;
    config->stdin_mode = false;
    config->json_output = false;
    config->threads = DEFAULT_THREADS;
}

// Free memory owned by the config.
void free_config(Config *config) {
    if (!config) {
        return;
    }
    free(config->root_path);
    config->root_path = NULL;
}
