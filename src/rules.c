

#include "rules.h"

#include <regex.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *name;
    const char *pattern;
    int flags;
    regex_t regex;
    bool compiled;
} RegexRule;

typedef struct {
    RegexRule *rules;
    size_t rule_count;
} RulesImpl;

static const RegexRule DEFAULT_RULES[] = {
    {"GENERIC_PASSWORD_KV", "password\\s*[:=]\\s*\\S+", REG_ICASE | REG_EXTENDED, {0}, false},
    {"GENERIC_APIKEY_KV", "api[_-]?key\\s*[:=]\\s*\\S+", REG_ICASE | REG_EXTENDED, {0}, false}
};

static void free_rules_impl(RulesImpl *rules_impl) {
    if (!rules_impl) {
        return;
    }

    for (size_t i = 0; i < rules_impl->rule_count; ++i) {
        if (rules_impl->rules[i].compiled) {
            regfree(&rules_impl->rules[i].regex);
        }
    }

    free(rules_impl->rules);
    free(rules_impl);
}

static int compile_rule(RegexRule *rule) {
    int result = regcomp(&rule->regex, rule->pattern, rule->flags);
    if (result != 0) {
        return -1;
    }
    rule->compiled = true;
    return 0;
}

int rules_init(RulesEngine *engine) {
    memset(engine, 0, sizeof(*engine));

    RulesImpl *rules_impl = calloc(1, sizeof(RulesImpl));
    if (!rules_impl) {
        return -1;
    }

    rules_impl->rule_count = sizeof(DEFAULT_RULES) / sizeof(DEFAULT_RULES[0]);
    rules_impl->rules = calloc(rules_impl->rule_count, sizeof(RegexRule));
    if (!rules_impl->rules) {
        free(rules_impl);
        return -1;
    }

    for (size_t i = 0; i < rules_impl->rule_count; ++i) {
        rules_impl->rules[i] = DEFAULT_RULES[i];
        if (compile_rule(&rules_impl->rules[i]) != 0) {
            free_rules_impl(rules_impl);
            return -1;
        }
    }

    engine->implementation = rules_impl;
    return 0;
}

void rules_destroy(RulesEngine *engine) {
    if (!engine || !engine->implementation) {
        return;
    }

    free_rules_impl((RulesImpl *)engine->implementation);
    engine->implementation = NULL;
}

void rules_scan_line(const RulesEngine *engine,
                     const char *line,
                     size_t length,
                     rules_match_callback callback,
                     void *user_data) {
    if (!engine || !engine->implementation || !line || !callback) {
        return;
    }

    const RulesImpl *rules_impl = (const RulesImpl *)engine->implementation;
    for (size_t i = 0; i < rules_impl->rule_count; ++i) {
        const RegexRule *rule = &rules_impl->rules[i];
        size_t offset = 0;

        while (offset <= length) {
            regmatch_t match;
            int result = regexec(&rule->regex, line + offset, 1, &match, 0);
            if (result != 0) {
                break;
            }

            size_t start = offset + (size_t)match.rm_so;
            size_t end = offset + (size_t)match.rm_eo;
            if (end <= start) {
                offset++;
                continue;
            }

            callback(rule->name, start, end, user_data);
            offset = end;
        }
    }
}
