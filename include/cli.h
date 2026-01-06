
#include <stdbool.h>

#define APP_NAME "SecretGuard"
#define APP_VERSION "0.1.0"
#define DEFAULT_MAX_DEPTH -1

typedef struct {
    char *root_path;
    int max_depth;
    bool stdin_mode;
} Config;

void init_config(Config *config);
void free_config(Config *config);
int parse_arguments(int argc, char **argv, Config *config);
void print_config(const Config *config);
void print_help(const char *program_name);

