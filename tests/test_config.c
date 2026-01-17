#include "unity.h"
#include "config.h"
#include "util.h"

void test_init_config_defaults(void) {
    Config config;
    init_config(&config);

    TEST_ASSERT_NULL(config.root_path);
    TEST_ASSERT_EQUAL_INT(DEFAULT_MAX_DEPTH, config.max_depth);
    TEST_ASSERT_FALSE(config.stdin_mode);
    TEST_ASSERT_FALSE(config.json_output);
    TEST_ASSERT_NULL(config.output_path);

    free_config(&config);
}

void test_init_config_null_is_safe(void) {
    init_config(NULL);
}

void test_free_config_clears_paths(void) {
    Config config;
    init_config(&config);
    config.root_path = duplicate_string("root");
    config.output_path = duplicate_string("report.txt");

    free_config(&config);

    TEST_ASSERT_NULL(config.root_path);
    TEST_ASSERT_NULL(config.output_path);
}

void test_free_config_null_is_safe(void) {
    free_config(NULL);
}

void run_config_tests(void) {
    RUN_TEST(test_init_config_defaults);
    RUN_TEST(test_init_config_null_is_safe);
    RUN_TEST(test_free_config_clears_paths);
    RUN_TEST(test_free_config_null_is_safe);
}
