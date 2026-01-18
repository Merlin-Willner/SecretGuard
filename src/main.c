#include "app.h"

#include "cli.h"
#include "rules.h"
#include "scanner_parallel.h"
#include "scanner.h"

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

    RulesEngine rules;
    if (rules_init(&rules) != 0) {
        fprintf(stderr, "ERROR: failed to initialize rules engine.\n");
        free_config(&config);
        return 1;
    }

    ScannerContext scanner;
    int exit_code = 0;
    if (scanner_scan_parallel(&config, &rules, &scanner) != 0) {
        fprintf(stderr, "ERROR: scanning failed.\n");
        exit_code = 1;
    }

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
