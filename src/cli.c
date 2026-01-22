#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static int parse_int(const char *text, int *out_value) {
    if (!text || !out_value) {
        return -1;
    }
    if (text[0] == '\0') {
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

int parse_arguments(int argc, char **argv, Config *config) {
    if (!config) {
        return 2;
    }
    int i = 1;
    const char *prog = (argc > 0) ? argv[0] : "app";

    while (i < argc) {
        const char *arg = argv[i];

        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            print_help(prog);
            return 1;
        } else if (strncmp(arg, "--max-depth", 11) == 0) {
            const char *value = NULL;
            if (strcmp(arg, "--max-depth") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "ERROR: --max-depth requires a value.\n");
                    return 2;
                }
                value = argv[++i];
            } else if (arg[11] == '=') {
                value = arg + 12;
            } else {
                fprintf(stderr, "ERROR: invalid --max-depth usage: %s\n", arg);
                return 2;
            }

            if (parse_int(value, &config->max_depth) != 0) {
                fprintf(stderr, "ERROR: invalid --max-depth value: %s\n", value ? value : "(null)");
                return 2;
            }
        } else if (strncmp(arg, "--threads", 9) == 0) {
            const char *value = NULL;
            if (strcmp(arg, "--threads") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "ERROR: --threads requires a value.\n");
                    return 2;
                }
                value = argv[++i];
            } else if (arg[9] == '=') {
                value = arg + 10;
            } else {
                fprintf(stderr, "ERROR: invalid --threads usage: %s\n", arg);
                return 2;
            }

            if (parse_int(value, &config->threads) != 0 || config->threads < 0) {
                fprintf(stderr, "ERROR: invalid --threads value: %s\n", value ? value : "(null)");
                return 2;
            }
        } else if (strcmp(arg, "--stdin") == 0) {
            config->stdin_mode = true;
        } else if (strcmp(arg, "--json") == 0) {
            config->json_output = true;
        } else if (strncmp(arg, "--out", 5) == 0) {
            const char *value = NULL;
            if (strcmp(arg, "--out") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "ERROR: --out requires a value.\n");
                    return 2;
                }
                value = argv[++i];
            } else if (arg[5] == '=') {
                value = arg + 6;
            } else {
                fprintf(stderr, "ERROR: invalid --out usage: %s\n", arg);
                return 2;
            }

            if (!value || value[0] == '\0') {
                fprintf(stderr, "ERROR: invalid --out value.\n");
                return 2;
            }
            if (config->output_path) {
                fprintf(stderr, "ERROR: multiple --out values provided.\n");
                return 2;
            }
            config->output_path = duplicate_string(value);
            if (!config->output_path) {
                fprintf(stderr, "ERROR: could not copy --out value.\n");
                return 2;
            }
        } else if (arg[0] == '-') {
            fprintf(stderr, "ERROR: unknown flag. Use --help to see valid options.\n");
            return 2;
        } else {
            if (!config->root_path) {
                config->root_path = duplicate_string(arg);
                if (!config->root_path) {
                    fprintf(stderr, "ERROR: could not copy path argument.\n");
                    return 2;
                }
            } else {
                fprintf(stderr, "ERROR: extra argument: %s\n", arg);
                return 2;
            }
        }

        i++;
    }

    if (config->stdin_mode && config->root_path) {
        fprintf(stderr, "ERROR: --stdin cannot be combined with a path.\n");
        return 2;
    }

    if (!config->root_path && !config->stdin_mode) {
        config->root_path = duplicate_string(".");
        if (!config->root_path) {
            fprintf(stderr, "ERROR: could not set default path.\n");
            return 2;
        }
    }

    return 0;
}

void print_config(const Config *config) {
    if (config->json_output) {
        return;
    }
    const char *root_path = config->root_path ? config->root_path : ".";
    const char *mode_label = config->stdin_mode ? "STDIN" : "filesystem";
    const char *target_label = root_path;
    if (config->stdin_mode) {
        target_label = "STDIN";
    } else if (strcmp(root_path, ".") == 0) {
        target_label = "Current Directory";
    }
    char depth_label[32];
    if (config->max_depth < 0) {
        snprintf(depth_label, sizeof(depth_label), "unlimited");
    } else {
        snprintf(depth_label, sizeof(depth_label), "%d", config->max_depth);
    }
    if (config->stdin_mode) {
        printf("%s v%s  \u2022  mode: %s\n", APP_NAME, APP_VERSION, mode_label);
    } else {
        char threads_label[32];
        if (config->threads <= 0) {
            snprintf(threads_label, sizeof(threads_label), "auto");
        } else {
            snprintf(threads_label, sizeof(threads_label), "%d", config->threads);
        }
        printf("%s v%s  \u2022  mode: %s \u2022  depth: %s\n",
               APP_NAME, APP_VERSION, mode_label, depth_label);
        printf("Threads: %s\n", threads_label);
    }
    printf("Target:  %s\n", target_label);
    if (config->output_path) {
        printf("Output:  %s\n", config->output_path);
    }
}

void print_help(const char *program_name) {
    printf("%s %s (Linux/WSL)\n", APP_NAME, APP_VERSION);
    printf("Usage: %s [OPTIONS] <path>\n", program_name);
    printf("Options:\n");
    printf("  -h, --help         Show this help text\n");
    printf("      --max-depth N  Limit how deep we recurse (default: -1 for unlimited)\n");
    printf("                     Example: %s --max-depth 3 path/to/scan\n", program_name);
    printf("      --threads N    Number of worker threads (default: 0 for auto)\n");
    printf("                     Example: %s --threads 4 path/to/scan\n", program_name);
    printf("      --stdin        Read from STDIN instead of a file path\n");
    printf("                     Example:\n");
    printf("                       %s --stdin <<'EOF'\n", program_name);
    printf("                       paste the text to check for security risks\n");
    printf("                       EOF\n");
    printf("      --json         Output results as JSON\n");
    printf("                     Example: %s --json path/to/scan\n", program_name);
    printf("      --out FILE     Write results to FILE instead of stdout\n");
    printf("                     Example: %s --out report.txt path/to/scan\n", program_name);
    printf("\nNote: Provide a path (default: current directory) or use --stdin.\n");
}
