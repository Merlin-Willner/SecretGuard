

#include "rules.h"

#include <regex.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *name;
    severity_t severity;
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
    {"GENERIC_PASSWORD_KV", SEVERITY_HIGH, "password[[:space:]]*[:=][[:space:]]*[^[:space:]]+", REG_ICASE | REG_EXTENDED, {0}, false},
    {"GENERIC_APIKEY_KV", SEVERITY_MEDIUM, "api[_-]?key[[:space:]]*[:=][[:space:]]*[^[:space:]]+", REG_ICASE | REG_EXTENDED, {0}, false},
    {"GENERIC_SECRET_KV", SEVERITY_HIGH, "secret[[:space:]]*[:=][[:space:]]*[^[:space:]]+", REG_ICASE | REG_EXTENDED, {0}, false},
    {"GENERIC_TOKEN_KV", SEVERITY_HIGH, "(access|refresh|id)?_?token[[:space:]]*[:=][[:space:]]*[^[:space:]]+", REG_ICASE | REG_EXTENDED, {0}, false},
    {"GENERIC_BEARER", SEVERITY_HIGH, "bearer[[:space:]]+[A-Za-z0-9._-]+", REG_ICASE | REG_EXTENDED, {0}, false},
    {"GENERIC_AUTH_KV", SEVERITY_MEDIUM, "auth(entication|orization)?[[:space:]]*[:=][[:space:]]*[^[:space:]]+", REG_ICASE | REG_EXTENDED, {0}, false},
    {"GENERIC_CLIENT_SECRET_KV", SEVERITY_HIGH, "client[_-]?secret[[:space:]]*[:=][[:space:]]*[^[:space:]]+", REG_ICASE | REG_EXTENDED, {0}, false},
    {"GENERIC_PRIVATE_KEY_PEM", SEVERITY_HIGH, "-----BEGIN[[:space:]]+(RSA|EC|DSA|OPENSSH)?[[:space:]]*PRIVATE[[:space:]]+KEY-----", REG_ICASE | REG_EXTENDED, {0}, false},

    {"GOOGLE_API_KEY", SEVERITY_HIGH, "AIza[0-9A-Za-z_-]{35}", REG_ICASE | REG_EXTENDED, {0}, false},
    {"GOOGLE_OAUTH_CLIENT_ID", SEVERITY_MEDIUM, "[0-9]+-[A-Za-z0-9_]+\\.apps\\.googleusercontent\\.com", REG_ICASE | REG_EXTENDED, {0}, false},
    {"GOOGLE_SERVICE_ACCOUNT_EMAIL", SEVERITY_MEDIUM, "[A-Za-z0-9._%+-]+@[^[:space:]]+\\.gserviceaccount\\.com", REG_ICASE | REG_EXTENDED, {0}, false},
    {"GOOGLE_SERVICE_ACCOUNT_KV", SEVERITY_HIGH, "\"type\"[[:space:]]*:[[:space:]]*\"service_account\"", REG_ICASE | REG_EXTENDED, {0}, false},
    {"GOOGLE_PRIVATE_KEY_ID_KV", SEVERITY_HIGH, "\"private_key_id\"[[:space:]]*:[[:space:]]*\"[A-Za-z0-9]+\"", REG_ICASE | REG_EXTENDED, {0}, false},

    {"FIREBASE_API_KEY_KV", SEVERITY_HIGH, "firebase[_-]?api[_-]?key[[:space:]]*[:=][[:space:]]*[^[:space:]]+", REG_ICASE | REG_EXTENDED, {0}, false},
    {"FIREBASE_DATABASE_URL", SEVERITY_MEDIUM, "https://[A-Za-z0-9-]+\\.(firebaseio\\.com|firebasedatabase\\.app)", REG_ICASE | REG_EXTENDED, {0}, false},
    {"FIREBASE_PROJECT_ID_KV", SEVERITY_MEDIUM, "project[_-]?id[[:space:]]*[:=][[:space:]]*[^[:space:]]+", REG_ICASE | REG_EXTENDED, {0}, false},
    {"FIREBASE_MESSAGING_SENDER_ID_KV", SEVERITY_MEDIUM, "messaging[_-]?sender[_-]?id[[:space:]]*[:=][[:space:]]*[^[:space:]]+", REG_ICASE | REG_EXTENDED, {0}, false},
    {"FIREBASE_APP_ID", SEVERITY_LOW, "app[_-]?id[[:space:]]*[:=][[:space:]]*1:[0-9]+:(android|ios|web):[A-Za-z0-9]+", REG_ICASE | REG_EXTENDED, {0}, false},
    {"FIREBASE_STORAGE_BUCKET", SEVERITY_LOW, "storage[_-]?bucket[[:space:]]*[:=][[:space:]]*[A-Za-z0-9._-]+\\.appspot\\.com", REG_ICASE | REG_EXTENDED, {0}, false},
    {"FIREBASE_MEASUREMENT_ID", SEVERITY_LOW, "measurement[_-]?id[[:space:]]*[:=][[:space:]]*G-[A-Za-z0-9]+", REG_ICASE | REG_EXTENDED, {0}, false},

    {"GOOGLE_ANALYTICS_ID", SEVERITY_LOW, "(UA-[0-9]{4,}-[0-9]+|G-[A-Za-z0-9]+)", REG_ICASE | REG_EXTENDED, {0}, false},

    {"GITHUB_TOKEN", SEVERITY_HIGH, "gh[opusr]_[A-Za-z0-9]{36,}", REG_ICASE | REG_EXTENDED, {0}, false},
    {"GITHUB_CLASSIC_TOKEN", SEVERITY_HIGH, "ghp_[A-Za-z0-9]{36,}", REG_ICASE | REG_EXTENDED, {0}, false},

    {"GITLAB_TOKEN", SEVERITY_HIGH, "glpat-[A-Za-z0-9_-]{20,}", REG_ICASE | REG_EXTENDED, {0}, false},

    {"SLACK_TOKEN", SEVERITY_HIGH, "xox[baprs]-[A-Za-z0-9-]{10,48}", REG_ICASE | REG_EXTENDED, {0}, false},

    {"JWT_TOKEN", SEVERITY_MEDIUM, "eyJ[A-Za-z0-9_-]+\\.[A-Za-z0-9._-]+\\.[A-Za-z0-9._-]+", REG_ICASE | REG_EXTENDED, {0}, false},

    {"AWS_ACCESS_KEY_ID", SEVERITY_HIGH, "AKIA[0-9A-Z]{16}", REG_ICASE | REG_EXTENDED, {0}, false},
    {"AWS_SECRET_ACCESS_KEY_KV", SEVERITY_HIGH, "aws[_-]?secret[_-]?access[_-]?key[[:space:]]*[:=][[:space:]]*[A-Za-z0-9/+=]{40}", REG_ICASE | REG_EXTENDED, {0}, false},

    {"DATABASE_URL_KV", SEVERITY_MEDIUM, "(database|db)[_-]?url[[:space:]]*[:=][[:space:]]*[^[:space:]]+", REG_ICASE | REG_EXTENDED, {0}, false},
    {"JDBC_URL", SEVERITY_MEDIUM, "jdbc:[A-Za-z0-9]+:[^[:space:]]+", REG_ICASE | REG_EXTENDED, {0}, false}
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

            callback(rule->name, rule->severity, start, end, user_data);
            offset = end;
        }
    }
}
