#include <stdio.h>

#include "cli.h"
#include "rules.h"
#include "scanner.h"
#include "walk.h"

/* Callback passed to the directory walker: scans a single file path. */
static int scan_file_callback(const char *path, void *user_data) {
    ScannerContext *scanner = (ScannerContext *)user_data;
    int res = scanner_scan_path(scanner, path);
    if (res != 0) {
        /* Log and continue walking; do not abort the whole traversal. */
        return 0;
    }
    return 0;
}

int main(int argc, char **argv) {
    Config config;
    init_config(&config);

    int parse_result = parse_arguments(argc, argv, &config);
    if (parse_result == 1) {
        free_config(&config);
        return 0;
    }
    if (parse_result != 0) {
        fprintf(stderr, "ERROR: could not read the arguments. Please fix the input and try again.\n");
        free_config(&config);
        return parse_result;
    }

    print_config(&config);

    /* Initialize rules engine and scanner. */
    RulesEngine rules;
    if (rules_init(&rules) != 0) {
        fprintf(stderr, "ERROR: failed to initialize rules engine.\n");
        free_config(&config);
        return 1;
    }

    ScannerContext scanner;
    scanner_init(&scanner, &rules);

    int exit_code = 0;
    FILE *out = stdout;
    if (config.output_path) {
        out = fopen(config.output_path, "w");
        if (!out) {
            fprintf(stderr, "ERROR: failed to open output file %s.\n", config.output_path);
            scanner_destroy(&scanner);
            rules_destroy(&rules);
            free_config(&config);
            return 1;
        }
    }

    if (config.stdin_mode) {
        /* Scan from standard input. */
        if (scanner_scan_stdin(&scanner) != 0) {
            fprintf(stderr, "ERROR: scanning stdin failed.\n");
            exit_code = 1;
        }
    } else {
        /* Recursively walk the root path and scan files. */
        int walk_result = walk_path(&config, scan_file_callback, &scanner);
        if (walk_result != 0) {
            fprintf(stderr, "ERROR: walking path failed.\n");
            exit_code = 1;
        }
    }

    if (config.json_output) {
        scanner_print_report_json(&scanner, out);
    } else {
        scanner_print_report(&scanner, out);
    }
    if (out != stdout) {
        fclose(out);
    }
    scanner_destroy(&scanner);
    rules_destroy(&rules);
    free_config(&config);
    return exit_code;
}
