#include "unity.h"
#include "scanner.h"
#include "test_utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void init_scanner(ScannerContext *scanner, RulesEngine *rules) {
    TEST_ASSERT_EQUAL_INT(0, rules_init(rules));
    scanner_init(scanner, rules);
}

static void destroy_scanner(ScannerContext *scanner, RulesEngine *rules) {
    scanner_destroy(scanner);
    rules_destroy(rules);
}

static char *create_temp_file(const char *root, const char *name, const char *content) {
    char *path = test_join_path(root, name);
    TEST_ASSERT_NOT_NULL(path);
    TEST_ASSERT_EQUAL_INT(0, test_write_file(path, content));
    return path;
}

static long find_index(const char *haystack, const char *needle) {
    const char *pos = strstr(haystack, needle);
    if (!pos) {
        return -1;
    }
    return (long)(pos - haystack);
}

static char *read_stream(FILE *stream) {
    TEST_ASSERT_NOT_NULL(stream);
    fflush(stream);
    long end = 0;
    TEST_ASSERT_EQUAL_INT(0, fseek(stream, 0, SEEK_END));
    end = ftell(stream);
    TEST_ASSERT_TRUE(end >= 0);
    TEST_ASSERT_EQUAL_INT(0, fseek(stream, 0, SEEK_SET));

    size_t size = (size_t)end;
    char *buffer = malloc(size + 1);
    TEST_ASSERT_NOT_NULL(buffer);
    size_t read_size = fread(buffer, 1, size, stream);
    buffer[read_size] = '\0';
    return buffer;
}

static char *capture_report(const ScannerContext *scanner, bool json) {
    FILE *temp = tmpfile();
    TEST_ASSERT_NOT_NULL(temp);
    if (json) {
        scanner_print_report_json(scanner, temp);
    } else {
        scanner_print_report(scanner, temp);
    }
    char *output = read_stream(temp);
    fclose(temp);
    return output;
}

static char *scan_content_capture_report(const char *content, bool json) {
    RulesEngine rules;
    ScannerContext scanner;
    init_scanner(&scanner, &rules);

    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);
    char *path = create_temp_file(root, "scan.txt", content);
    TEST_ASSERT_EQUAL_INT(0, scanner_scan_path(&scanner, path));

    char *output = capture_report(&scanner, json);

    free(path);
    test_remove_tree(root);
    free(root);
    destroy_scanner(&scanner, &rules);
    return output;
}

static char *create_findings_fixture(void) {
    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);

    char *root_file = create_temp_file(root, "root.txt", "password = hunter2\n");
    char *dir_a = test_join_path(root, "dirA");
    TEST_ASSERT_EQUAL_INT(0, test_make_dir(dir_a));
    char *file_a = create_temp_file(dir_a, "a1.txt",
                                    "aws_secret_access_key = AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
    char *dir_b = test_join_path(dir_a, "dirB");
    TEST_ASSERT_EQUAL_INT(0, test_make_dir(dir_b));
    char *file_b = create_temp_file(dir_b, "b1.txt", "bearer abcdefghijklmno\n");

    free(root_file);
    free(dir_a);
    free(file_a);
    free(dir_b);
    free(file_b);
    return root;
}

void test_scan_file_detects_high(void) {
    RulesEngine rules;
    ScannerContext scanner;
    init_scanner(&scanner, &rules);

    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);
    char *path = create_temp_file(root, "secrets.txt",
                                  "AKIA1234567890ABCDE1\n"
                                  "aws_secret_access_key = AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");

    TEST_ASSERT_EQUAL_INT(0, scanner_scan_path(&scanner, path));
    TEST_ASSERT_TRUE(scanner.finding_count >= 2);
    TEST_ASSERT_EQUAL(SEVERITY_HIGH, scanner.highest_severity);
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scanner.files_scanned);
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned int)scanner.files_skipped);

    free(path);
    test_remove_tree(root);
    free(root);
    destroy_scanner(&scanner, &rules);
}

void test_scan_file_no_secrets(void) {
    RulesEngine rules;
    ScannerContext scanner;
    init_scanner(&scanner, &rules);

    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);
    char *path = create_temp_file(root, "plain.txt", "hello world\n");

    TEST_ASSERT_EQUAL_INT(0, scanner_scan_path(&scanner, path));
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned int)scanner.finding_count);
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scanner.files_scanned);

    free(path);
    test_remove_tree(root);
    free(root);
    destroy_scanner(&scanner, &rules);
}

void test_scan_file_without_newline_detects_secret(void) {
    RulesEngine rules;
    ScannerContext scanner;
    init_scanner(&scanner, &rules);

    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);
    char *path = create_temp_file(root, "no_newline.txt", "password = hunter2");

    TEST_ASSERT_EQUAL_INT(0, scanner_scan_path(&scanner, path));
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scanner.finding_count);
    TEST_ASSERT_EQUAL(SEVERITY_HIGH, scanner.highest_severity);

    free(path);
    test_remove_tree(root);
    free(root);
    destroy_scanner(&scanner, &rules);
}

void test_scan_stdin_detects_secret(void) {
    RulesEngine rules;
    ScannerContext scanner;
    init_scanner(&scanner, &rules);

    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);
    char *path = create_temp_file(root, "stdin.txt", "password = hunter2\n");

    int saved_fd = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stdin(path, &saved_fd));
    TEST_ASSERT_EQUAL_INT(0, scanner_scan_stdin(&scanner));
    test_restore_stdin(saved_fd);

    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scanner.finding_count);
    TEST_ASSERT_EQUAL(SEVERITY_HIGH, scanner.highest_severity);
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scanner.files_scanned);

    free(path);
    test_remove_tree(root);
    free(root);
    destroy_scanner(&scanner, &rules);
}

void test_scan_stdin_empty_input(void) {
    RulesEngine rules;
    ScannerContext scanner;
    init_scanner(&scanner, &rules);

    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);
    char *path = create_temp_file(root, "stdin.txt", "");

    int saved_fd = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stdin(path, &saved_fd));
    TEST_ASSERT_EQUAL_INT(0, scanner_scan_stdin(&scanner));
    test_restore_stdin(saved_fd);

    TEST_ASSERT_EQUAL_UINT(0u, (unsigned int)scanner.finding_count);
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scanner.files_scanned);
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned int)scanner.files_skipped);
    TEST_ASSERT_EQUAL(SEVERITY_LOW, scanner.highest_severity);

    free(path);
    test_remove_tree(root);
    free(root);
    destroy_scanner(&scanner, &rules);
}

void test_scan_path_missing_file_is_skipped(void) {
    RulesEngine rules;
    ScannerContext scanner;
    init_scanner(&scanner, &rules);

    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);
    char *path = test_join_path(root, "missing.txt");
    TEST_ASSERT_NOT_NULL(path);

    int saved_stderr = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stderr_to_null(&saved_stderr));

    TEST_ASSERT_EQUAL_INT(-1, scanner_scan_path(&scanner, path));

    test_restore_stderr(saved_stderr);
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned int)scanner.finding_count);
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned int)scanner.files_scanned);
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scanner.files_skipped);

    free(path);
    test_remove_tree(root);
    free(root);
    destroy_scanner(&scanner, &rules);
}

void test_scan_path_binary_is_skipped(void) {
    RulesEngine rules;
    ScannerContext scanner;
    init_scanner(&scanner, &rules);

    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);
    char *path = test_join_path(root, "binary.bin");
    TEST_ASSERT_NOT_NULL(path);

    const unsigned char data[] = {0x00, 0x01, 0x02, 'A', '\n'};
    TEST_ASSERT_EQUAL_INT(0, test_write_file_bytes(path, data, sizeof(data)));

    TEST_ASSERT_EQUAL_INT(0, scanner_scan_path(&scanner, path));
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned int)scanner.finding_count);
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned int)scanner.files_scanned);
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)scanner.files_skipped);

    free(path);
    test_remove_tree(root);
    free(root);
    destroy_scanner(&scanner, &rules);
}

void test_report_no_color_for_file(void) {
    RulesEngine rules;
    ScannerContext scanner;
    init_scanner(&scanner, &rules);

    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);
    char *path = create_temp_file(root, "secrets.txt", "password = hunter2\n");
    TEST_ASSERT_EQUAL_INT(0, scanner_scan_path(&scanner, path));

    char *output = capture_report(&scanner, false);
    TEST_ASSERT_NOT_NULL(strstr(output, "Summary:"));
    TEST_ASSERT_NOT_NULL(strstr(output, "Results:"));
    TEST_ASSERT_NOT_NULL(strstr(output, "file:"));
    TEST_ASSERT_NULL(strstr(output, "\x1b["));

    free(output);
    free(path);
    test_remove_tree(root);
    free(root);
    destroy_scanner(&scanner, &rules);
}

void test_report_no_findings_status_ok(void) {
    RulesEngine rules;
    ScannerContext scanner;
    init_scanner(&scanner, &rules);

    char *output = capture_report(&scanner, false);
    TEST_ASSERT_NOT_NULL(strstr(output, "Summary:"));
    TEST_ASSERT_NOT_NULL(strstr(output, "OK"));
    TEST_ASSERT_NOT_NULL(strstr(output, "(no findings)"));

    free(output);
    destroy_scanner(&scanner, &rules);
}

void test_report_status_warn(void) {
    char *output = scan_content_capture_report("api_key = ABCD\n", false);
    TEST_ASSERT_NOT_NULL(strstr(output, "WARN"));
    TEST_ASSERT_NULL(strstr(output, "ERROR"));
    free(output);
}

void test_report_status_error(void) {
    char *output = scan_content_capture_report("password = hunter2\n", false);
    TEST_ASSERT_NOT_NULL(strstr(output, "ERROR"));
    free(output);
}

void test_report_order_by_severity(void) {
    RulesEngine rules;
    ScannerContext scanner;
    init_scanner(&scanner, &rules);

    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);
    char *medium_path = create_temp_file(root, "medium.txt", "api_key = ABCD\n");
    char *high_path = create_temp_file(root, "high.txt", "password = hunter2\n");

    TEST_ASSERT_EQUAL_INT(0, scanner_scan_path(&scanner, medium_path));
    TEST_ASSERT_EQUAL_INT(0, scanner_scan_path(&scanner, high_path));

    char *output = capture_report(&scanner, false);
    long high_index = find_index(output, "[HIGH]");
    long medium_index = find_index(output, "[MEDIUM]");
    TEST_ASSERT_TRUE(high_index >= 0);
    TEST_ASSERT_TRUE(medium_index >= 0);
    TEST_ASSERT_TRUE(high_index < medium_index);

    free(output);
    free(medium_path);
    free(high_path);
    test_remove_tree(root);
    free(root);
    destroy_scanner(&scanner, &rules);
}

void test_report_json_output(void) {
    RulesEngine rules;
    ScannerContext scanner;
    init_scanner(&scanner, &rules);

    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);
    char *path = create_temp_file(root, "secrets.txt", "password = hunter2\n");
    TEST_ASSERT_EQUAL_INT(0, scanner_scan_path(&scanner, path));

    char *output = capture_report(&scanner, true);
    TEST_ASSERT_NOT_NULL(strstr(output, "\"summary\""));
    TEST_ASSERT_NOT_NULL(strstr(output, "\"findings\":1"));
    TEST_ASSERT_NULL(strstr(output, "\x1b["));

    free(output);
    free(path);
    test_remove_tree(root);
    free(root);
    destroy_scanner(&scanner, &rules);
}

void test_report_json_status_values(void) {
    RulesEngine rules;
    ScannerContext scanner;
    init_scanner(&scanner, &rules);

    char *ok_output = capture_report(&scanner, true);
    TEST_ASSERT_NOT_NULL(strstr(ok_output, "\"status\":\"OK\""));
    free(ok_output);
    destroy_scanner(&scanner, &rules);

    char *warn_output = scan_content_capture_report("api_key = ABCD\n", true);
    TEST_ASSERT_NOT_NULL(strstr(warn_output, "\"status\":\"WARN\""));
    free(warn_output);

    char *error_output = scan_content_capture_report("password = hunter2\n", true);
    TEST_ASSERT_NOT_NULL(strstr(error_output, "\"status\":\"ERROR\""));
    free(error_output);
}

void test_findings_depth_counts(void) {
    char *root = create_findings_fixture();
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)count_findings_with_depth(root, 0));
    TEST_ASSERT_EQUAL_UINT(2u, (unsigned int)count_findings_with_depth(root, 1));
    TEST_ASSERT_EQUAL_UINT(3u, (unsigned int)count_findings_with_depth(root, 2));
    TEST_ASSERT_EQUAL_UINT(3u, (unsigned int)count_findings_with_depth(root, -1));
    test_remove_tree(root);
    free(root);
}

void run_scanner_tests(void) {
    RUN_TEST(test_scan_file_detects_high);
    RUN_TEST(test_scan_file_no_secrets);
    RUN_TEST(test_scan_file_without_newline_detects_secret);
    RUN_TEST(test_scan_stdin_detects_secret);
    RUN_TEST(test_scan_stdin_empty_input);
    RUN_TEST(test_scan_path_missing_file_is_skipped);
    RUN_TEST(test_scan_path_binary_is_skipped);
    RUN_TEST(test_report_no_color_for_file);
    RUN_TEST(test_report_no_findings_status_ok);
    RUN_TEST(test_report_status_warn);
    RUN_TEST(test_report_status_error);
    RUN_TEST(test_report_order_by_severity);
    RUN_TEST(test_report_json_output);
    RUN_TEST(test_report_json_status_values);
    RUN_TEST(test_findings_depth_counts);
}
