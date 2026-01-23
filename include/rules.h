#ifndef RULES_H
#define RULES_H

#include <stddef.h>

typedef struct RulesEngine RulesEngine;

typedef enum {
    SEVERITY_LOW = 0,
    SEVERITY_MEDIUM,
    SEVERITY_HIGH
} severity_t;

// Called for each match found in a line.
// start/end are byte offsets in the line.
typedef void (*rules_match_callback)(const char *rule_name,
                                     severity_t severity,
                                     size_t start,
                                     size_t end,
                                     void *user_data);

struct RulesEngine {
    void *implementation;
};

// Initialize and compile the default rules. Returns 0 on success.
int rules_init(RulesEngine *engine);

// Free the rules engine.
void rules_destroy(RulesEngine *engine);

// Scan one line and invoke the callback for each match.
void rules_scan_line(const RulesEngine *engine,
                     const char *line,
                     size_t length,
                     rules_match_callback callback,
                     void *user_data);

#endif
