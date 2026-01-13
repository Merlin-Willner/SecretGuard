#ifndef RULES_H
#define RULES_H

#include <stddef.h>

typedef struct RulesEngine RulesEngine;

typedef void (*rules_match_callback)(const char *rule_name,
                                     size_t start,
                                     size_t end,
                                     void *user_data);

struct RulesEngine {
    void *implementation;
};

int rules_init(RulesEngine *engine);

void rules_destroy(RulesEngine *engine);

void rules_scan_line(const RulesEngine *engine,
                     const char *line,
                     size_t length,
                     rules_match_callback callback,
                     void *user_data);

#endif
