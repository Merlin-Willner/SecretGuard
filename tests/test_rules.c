#include "unity.h"
#include "rules.h"

#include <string.h>

typedef struct {
    const char *name;
    size_t count;
} RuleCount;

typedef struct {
    RuleCount *rules;
    size_t rule_count;
    size_t total;
} MatchState;

static void match_callback(const char *rule_name,
                           severity_t severity,
                           size_t start,
                           size_t end,
                           void *user_data) {
    (void)severity;
    (void)start;
    (void)end;
    MatchState *state = (MatchState *)user_data;
    state->total++;
    for (size_t i = 0; i < state->rule_count; ++i) {
        if (strcmp(rule_name, state->rules[i].name) == 0) {
            state->rules[i].count++;
            break;
        }
    }
}

static size_t scan_total(const char *line) {
    RulesEngine engine;
    TEST_ASSERT_EQUAL_INT(0, rules_init(&engine));

    MatchState state = {NULL, 0, 0};
    rules_scan_line(&engine, line, strlen(line), match_callback, &state);
    rules_destroy(&engine);
    return state.total;
}

static size_t scan_for_rule(const char *line, const char *rule_name, size_t *total) {
    RulesEngine engine;
    TEST_ASSERT_EQUAL_INT(0, rules_init(&engine));

    RuleCount rule = {rule_name, 0};
    MatchState state = {&rule, 1, 0};
    rules_scan_line(&engine, line, strlen(line), match_callback, &state);
    rules_destroy(&engine);
    if (total) {
        *total = state.total;
    }
    return rule.count;
}

void test_rules_detect_google_api_key(void) {
    const char *line = "key=\"AIza01234567890123456789012345678901234\"";
    size_t total = 0;
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scan_for_rule(line, "GOOGLE_API_KEY", &total));
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)total);
}

void test_rules_detect_aws_access_key_id(void) {
    const char *line = "AKIA1234567890ABCDE1";
    size_t total = 0;
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scan_for_rule(line, "AWS_ACCESS_KEY_ID", &total));
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)total);
}

void test_rules_detect_aws_secret_access_key_kv(void) {
    const char *line = "aws_secret_access_key = AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    size_t total = 0;
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scan_for_rule(line, "AWS_SECRET_ACCESS_KEY_KV", &total));
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)total);
}

void test_rules_detect_jwt_token(void) {
    const char *line = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIn0.signature";
    size_t total = 0;
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scan_for_rule(line, "JWT_TOKEN", &total));
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)total);
}

void test_rules_detect_sendgrid_key(void) {
    const char *line = "SG.abcdefghijklmnop.qrstuvwxyzABCDEFGH";
    size_t total = 0;
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scan_for_rule(line, "SENDGRID_API_KEY", &total));
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)total);
}

void test_rules_detect_generic_bearer(void) {
    const char *line = "bearer abcdefghijklmno";
    size_t total = 0;
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scan_for_rule(line, "GENERIC_BEARER", &total));
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)total);
}

void test_rules_ignore_bearer_in_word(void) {
    const char *line = "talebearer abcdefghijklmno";
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned int)scan_total(line));
}

void test_rules_ignore_short_secret_value(void) {
    const char *line = "secret: short";
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned int)scan_total(line));
}

void test_rules_detect_generic_password_kv(void) {
    const char *line = "password = Jesus";
    size_t total = 0;
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scan_for_rule(line, "GENERIC_PASSWORD_KV", &total));
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)total);
}

void test_rules_detect_generic_token_kv(void) {
    const char *line = "access_token: abcdefghijklmnop";
    size_t total = 0;
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scan_for_rule(line, "GENERIC_TOKEN_KV", &total));
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)total);
}

void test_rules_ignore_password_in_word(void) {
    const char *line = "mypassword=hidden";
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned int)scan_total(line));
}

void test_rules_ignore_short_bearer_token(void) {
    const char *line = "bearer short";
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned int)scan_total(line));
}

void test_rules_empty_string_no_matches(void) {
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned int)scan_total(""));
}

void run_rules_tests(void) {
    RUN_TEST(test_rules_detect_google_api_key);
    RUN_TEST(test_rules_detect_aws_access_key_id);
    RUN_TEST(test_rules_detect_aws_secret_access_key_kv);
    RUN_TEST(test_rules_detect_jwt_token);
    RUN_TEST(test_rules_detect_sendgrid_key);
    RUN_TEST(test_rules_detect_generic_bearer);
    RUN_TEST(test_rules_ignore_bearer_in_word);
    RUN_TEST(test_rules_ignore_short_secret_value);
    RUN_TEST(test_rules_detect_generic_password_kv);
    RUN_TEST(test_rules_detect_generic_token_kv);
    RUN_TEST(test_rules_ignore_password_in_word);
    RUN_TEST(test_rules_ignore_short_bearer_token);
    RUN_TEST(test_rules_empty_string_no_matches);
}
