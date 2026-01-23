
#ifndef CLI_H
#define CLI_H

#include "config.h"

int parse_arguments(int argc, char **argv, Config *config);
void print_config(const Config *config);
void print_help(const char *program_name);

#endif /* CLI_H */
