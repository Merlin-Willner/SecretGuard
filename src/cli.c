#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Make a heap copy of a C string.
static char *duplicate_string(const char *text) {
    if (!text) {
        return NULL;
    }
    size_t length = strlen(text) + 1;
    char *copy = malloc(length);
    if (copy) {
        memcpy(copy, text, length);
    }
    return copy;
}

// Parse a full integer (no extra characters).
static int parse_int(const char *text, int *out_value) {
    if (!text || !out_value) {
        return -1;
    }
    char *end_ptr = NULL;
    long parsed = strtol(text, &end_ptr, 10);
    if (!end_ptr || *end_ptr != '\0') {
        return -1;
    }
    *out_value = (int)parsed;
    return 0;
}

// Parse CLI args into the config.
// Returns: 0 = ok, 1 = help shown, 2 = error.
int parse_arguments(int argc, char **argv, Config *config) {
    int i = 1;
    const char *prog = (argc > 0) ? argv[0] : "app";

    while (i < argc) {
        const char *arg = argv[i];

        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            print_help(prog);
            return 1;
        } else if (strncmp(arg, "--max-depth", 11) == 0) {
            // Supports "--max-depth 3" and "--max-depth=3".
            const char *value = NULL;
            if (strcmp(arg, "--max-depth") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "Invalid --max-depth value: (missing)\n");
                    return 2;
                }
                value = argv[++i];
            } else if (arg[11] == '=') {
                value = arg + 12;
            } else {
                fprintf(stderr, "Invalid --max-depth usage: %s\n", arg);
                return 2;
            }

            if (parse_int(value, &config->max_depth) != 0) {
                fprintf(stderr, "Invalid --max-depth value: %s\n", value ? value : "(null)");
                return 2;
            }
            printf("Max depth set to %d\n", config->max_depth);
        } else if (strcmp(arg, "--stdin") == 0) {
            config->stdin_mode = true;
            printf("Reading from stdin instead of a path\n");
        } else if (arg[0] == '-') {
            fprintf(stderr, "Unknown flag. Use --help to see valid options.\n");
            return 2;
        } else {
            // Only one path is allowed.
            if (!config->root_path) {
                config->root_path = duplicate_string(arg);
                if (!config->root_path) {
                    fprintf(stderr, "Could not copy path argument\n");
                    return 2;
                }
                printf("Root path set to: %s\n", config->root_path);
            } else {
                fprintf(stderr, "Extra argument detected: %s\n", arg);
                return 2;
            }
        }

        i++;
    }

    // Need either a path or stdin.
    if (!config->root_path && !config->stdin_mode) {
        fprintf(stderr, "Please provide a path or --stdin so we have input to scan.\n");
        return 2;
    }

    return 0;
}

// Print the final config.
void print_config(const Config *config) {
    printf("\n--- Current configuration ---\n");
    printf("Program : %s %s\n", APP_NAME, APP_VERSION);
    printf("Root path: %s\n", config->root_path ? config->root_path : "(none, stdin only)");
    printf("Max depth: %d\n", config->max_depth);
    printf("Use stdin : %s\n", config->stdin_mode ? "yes" : "no");
    printf("-----------------------------\n\n");
}

// Show usage info.
void print_help(const char *program_name) {
    printf("%s - Increment 1: basic main and flags\n", APP_NAME);
    printf("Usage: %s [OPTIONS] <path>\n", program_name);
    printf("Options:\n");
    printf("  -h, --help         Show this help text\n");
    printf("      --max-depth N  Limit how deep we recurse (default: -1 for unlimited)\n");
    printf("      --stdin        Read from STDIN instead of a file path\n");
    printf("\nNote: Provide either a path or --stdin.\n");
}
