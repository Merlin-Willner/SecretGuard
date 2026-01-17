#include "unity.h"
#include "cli.h"
#include "config.h"
#include "test_utils.h"

static void init_cli_config(Config *config) {
    init_config(config);
}

static void destroy_cli_config(Config *config) {
    free_config(config);
}

void test_parse_help_short_flag(void) {
    int saved_stdout = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stdout_to_null(&saved_stdout));

    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "-h"};
    TEST_ASSERT_EQUAL_INT(1, parse_arguments(2, argv, &config));
    destroy_cli_config(&config);

    test_restore_stdout(saved_stdout);
}

void test_parse_help_long_flag(void) {
    int saved_stdout = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stdout_to_null(&saved_stdout));

    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "--help"};
    TEST_ASSERT_EQUAL_INT(1, parse_arguments(2, argv, &config));
    destroy_cli_config(&config);

    test_restore_stdout(saved_stdout);
}

void test_parse_max_depth_space_value(void) {
    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "--max-depth", "2", "scan-target"};
    TEST_ASSERT_EQUAL_INT(0, parse_arguments(4, argv, &config));
    TEST_ASSERT_EQUAL_INT(2, config.max_depth);
    TEST_ASSERT_NOT_NULL(config.root_path);
    TEST_ASSERT_EQUAL_STRING("scan-target", config.root_path);
    destroy_cli_config(&config);
}

void test_parse_max_depth_equals_value(void) {
    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "--max-depth=3", "scan-target"};
    TEST_ASSERT_EQUAL_INT(0, parse_arguments(3, argv, &config));
    TEST_ASSERT_EQUAL_INT(3, config.max_depth);
    destroy_cli_config(&config);
}

void test_parse_max_depth_invalid_value(void) {
    int saved_stderr = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stderr_to_null(&saved_stderr));

    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "--max-depth", "nope"};
    TEST_ASSERT_EQUAL_INT(2, parse_arguments(3, argv, &config));
    destroy_cli_config(&config);

    test_restore_stderr(saved_stderr);
}

void test_parse_stdin_flag(void) {
    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "--stdin"};
    TEST_ASSERT_EQUAL_INT(0, parse_arguments(2, argv, &config));
    TEST_ASSERT_TRUE(config.stdin_mode);
    TEST_ASSERT_NULL(config.root_path);
    destroy_cli_config(&config);
}

void test_parse_stdin_with_path_is_error(void) {
    int saved_stderr = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stderr_to_null(&saved_stderr));

    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "--stdin", "scan-target"};
    TEST_ASSERT_EQUAL_INT(2, parse_arguments(3, argv, &config));
    destroy_cli_config(&config);

    test_restore_stderr(saved_stderr);
}

void test_parse_json_flag(void) {
    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "--json", "scan-target"};
    TEST_ASSERT_EQUAL_INT(0, parse_arguments(3, argv, &config));
    TEST_ASSERT_TRUE(config.json_output);
    destroy_cli_config(&config);
}

void test_parse_out_flag_space_value(void) {
    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "--out", "report.json", "scan-target"};
    TEST_ASSERT_EQUAL_INT(0, parse_arguments(4, argv, &config));
    TEST_ASSERT_NOT_NULL(config.output_path);
    TEST_ASSERT_EQUAL_STRING("report.json", config.output_path);
    destroy_cli_config(&config);
}

void test_parse_out_flag_equals_value(void) {
    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "--out=report.json", "scan-target"};
    TEST_ASSERT_EQUAL_INT(0, parse_arguments(3, argv, &config));
    TEST_ASSERT_NOT_NULL(config.output_path);
    TEST_ASSERT_EQUAL_STRING("report.json", config.output_path);
    destroy_cli_config(&config);
}

void test_parse_out_duplicate_value(void) {
    int saved_stderr = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stderr_to_null(&saved_stderr));

    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "--out=one", "--out=two"};
    TEST_ASSERT_EQUAL_INT(2, parse_arguments(3, argv, &config));
    destroy_cli_config(&config);

    test_restore_stderr(saved_stderr);
}

void test_parse_out_empty_value(void) {
    int saved_stderr = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stderr_to_null(&saved_stderr));

    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "--out="};
    TEST_ASSERT_EQUAL_INT(2, parse_arguments(2, argv, &config));
    destroy_cli_config(&config);

    test_restore_stderr(saved_stderr);
}

void test_parse_max_depth_empty_value(void) {
    int saved_stderr = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stderr_to_null(&saved_stderr));

    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "--max-depth="};
    TEST_ASSERT_EQUAL_INT(2, parse_arguments(2, argv, &config));
    destroy_cli_config(&config);

    test_restore_stderr(saved_stderr);
}

void test_parse_unknown_flag(void) {
    int saved_stderr = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stderr_to_null(&saved_stderr));

    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "--nope"};
    TEST_ASSERT_EQUAL_INT(2, parse_arguments(2, argv, &config));
    destroy_cli_config(&config);

    test_restore_stderr(saved_stderr);
}

void test_parse_extra_argument(void) {
    int saved_stderr = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stderr_to_null(&saved_stderr));

    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard", "one", "two"};
    TEST_ASSERT_EQUAL_INT(2, parse_arguments(3, argv, &config));
    destroy_cli_config(&config);

    test_restore_stderr(saved_stderr);
}

void test_parse_default_root_path(void) {
    Config config;
    init_cli_config(&config);
    char *argv[] = {"secretguard"};
    TEST_ASSERT_EQUAL_INT(0, parse_arguments(1, argv, &config));
    TEST_ASSERT_NOT_NULL(config.root_path);
    TEST_ASSERT_EQUAL_STRING(".", config.root_path);
    destroy_cli_config(&config);
}

void run_cli_tests(void) {
    RUN_TEST(test_parse_help_short_flag);
    RUN_TEST(test_parse_help_long_flag);
    RUN_TEST(test_parse_max_depth_space_value);
    RUN_TEST(test_parse_max_depth_equals_value);
    RUN_TEST(test_parse_max_depth_invalid_value);
    RUN_TEST(test_parse_stdin_flag);
    RUN_TEST(test_parse_stdin_with_path_is_error);
    RUN_TEST(test_parse_json_flag);
    RUN_TEST(test_parse_out_flag_space_value);
    RUN_TEST(test_parse_out_flag_equals_value);
    RUN_TEST(test_parse_out_duplicate_value);
    RUN_TEST(test_parse_out_empty_value);
    RUN_TEST(test_parse_max_depth_empty_value);
    RUN_TEST(test_parse_unknown_flag);
    RUN_TEST(test_parse_extra_argument);
    RUN_TEST(test_parse_default_root_path);
}
