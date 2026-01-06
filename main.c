#include <stdio.h>

#include "include/cli.h"

int main(int argc, char **argv) {
    Config config;
    init_config(&config);

    printf("== SecretGuard ==\n");
    printf("We only parse input flags for now \n\n");

    int parse_result = parse_arguments(argc, argv, &config);
    if (parse_result == 1) {
        /* Help or version was shown; nothing else to do. */
        free_config(&config);
        return 0;
    }
    if (parse_result != 0) {
        fprintf(stderr, "Could not read the arguments. Please fix the input and try again.\n");
        free_config(&config);
        return parse_result;
    }

    print_config(&config);

    /* This is where the scanner will run in later increments.
       For now we just confirm that the CLI parsing works. */
    printf("Scanner not implemented yet. This step is only about inputs.\n");

    // Uncomment to check that the program reaches this point:
    // printf("[debug] Ready to start scanning in the next increment.\\n");

    free_config(&config);
    return 0;
}
