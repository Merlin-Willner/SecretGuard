#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define APP_NAME "SecretGuard"
#define APP_VERSION "0.1.0"
#define DEFAULT_MAX_DEPTH -1
#define DEFAULT_THREADS 0

typedef struct {
    char *root_path;
    int max_depth;
    bool stdin_mode;
    bool json_output;
    int threads;
    char *output_path;
} Config;

void init_config(Config *config);
void free_config(Config *config);

/* Friendly label used when scanning standard input. */
#define DEFAULT_STDIN_LABEL "stdin"

#endif /* CONFIG_H */
