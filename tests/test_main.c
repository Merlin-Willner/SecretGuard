#include "unity.h"
#include "app.h"
#include "test_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_file(const char *path) {
    FILE *file = fopen(path, "r");
    TEST_ASSERT_NOT_NULL(file);
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    TEST_ASSERT_TRUE(size >= 0);
    fseek(file, 0, SEEK_SET);
    char *buffer = malloc((size_t)size + 1);
    TEST_ASSERT_NOT_NULL(buffer);
    size_t read_size = fread(buffer, 1, (size_t)size, file);
    buffer[read_size] = '\0';
    fclose(file);
    return buffer;
}

void test_app_run_writes_json_file(void) {
    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);

    char *file = test_join_path(root, "secrets.txt");
    TEST_ASSERT_EQUAL_INT(0, test_write_file(file, "password = hunter2\n"));
    char *out_path = test_join_path(root, "report.json");

    int saved_stdout = -1;
    int saved_stderr = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stdout_to_null(&saved_stdout));
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stderr_to_null(&saved_stderr));

    char *argv[] = {"secretguard", "--json", "--out", out_path, root};
    TEST_ASSERT_EQUAL_INT(0, app_run(5, argv));

    test_restore_stdout(saved_stdout);
    test_restore_stderr(saved_stderr);

    char *output = read_file(out_path);
    TEST_ASSERT_NOT_NULL(strstr(output, "\"summary\""));
    TEST_ASSERT_NOT_NULL(strstr(output, "\"findings\":1"));

    free(output);
    free(file);
    free(out_path);
    test_remove_tree(root);
    free(root);
}

void test_app_run_stdin_json_output(void) {
    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);

    char *stdin_path = test_join_path(root, "stdin.txt");
    TEST_ASSERT_EQUAL_INT(0, test_write_file(stdin_path, "password = hunter2\n"));
    char *out_path = test_join_path(root, "stdin_report.json");

    int saved_stdin = -1;
    int saved_stdout = -1;
    int saved_stderr = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stdin(stdin_path, &saved_stdin));
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stdout_to_null(&saved_stdout));
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stderr_to_null(&saved_stderr));

    char *argv[] = {"secretguard", "--stdin", "--json", "--out", out_path};
    TEST_ASSERT_EQUAL_INT(0, app_run(5, argv));

    test_restore_stdin(saved_stdin);
    test_restore_stdout(saved_stdout);
    test_restore_stderr(saved_stderr);

    char *output = read_file(out_path);
    TEST_ASSERT_NOT_NULL(strstr(output, "\"findings\":1"));

    free(output);
    free(stdin_path);
    free(out_path);
    test_remove_tree(root);
    free(root);
}

void test_app_run_invalid_args_returns_error(void) {
    int saved_stderr = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stderr_to_null(&saved_stderr));

    char *argv[] = {"secretguard", "--max-depth"};
    TEST_ASSERT_EQUAL_INT(2, app_run(2, argv));

    test_restore_stderr(saved_stderr);
}

void run_app_tests(void) {
    RUN_TEST(test_app_run_writes_json_file);
    RUN_TEST(test_app_run_stdin_json_output);
    RUN_TEST(test_app_run_invalid_args_returns_error);
}
