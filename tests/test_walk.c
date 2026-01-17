#include "unity.h"
#include "walk.h"
#include "config.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

static int count_files_callback(const char *path, void *user_data) {
    (void)path;
    if (!user_data) {
        return 0;
    }
    size_t *count = (size_t *)user_data;
    (*count)++;
    return 0;
}

static size_t count_files_with_depth(const char *root_path, int max_depth) {
    Config config;
    init_walk_config(&config, root_path, max_depth);
    size_t count = 0;
    TEST_ASSERT_EQUAL_INT(0, walk_path(&config, count_files_callback, &count));
    free_config(&config);
    return count;
}

static char *create_walk_fixture(void) {
    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);

    char *root_file1 = test_join_path(root, "root1.txt");
    char *root_file2 = test_join_path(root, "root2.txt");
    TEST_ASSERT_EQUAL_INT(0, test_write_file(root_file1, "root"));
    TEST_ASSERT_EQUAL_INT(0, test_write_file(root_file2, "root"));

    char *dir_a = test_join_path(root, "dirA");
    TEST_ASSERT_EQUAL_INT(0, test_make_dir(dir_a));
    char *file_a = test_join_path(dir_a, "a1.txt");
    TEST_ASSERT_EQUAL_INT(0, test_write_file(file_a, "a1"));

    char *dir_b = test_join_path(dir_a, "dirB");
    TEST_ASSERT_EQUAL_INT(0, test_make_dir(dir_b));
    char *file_b = test_join_path(dir_b, "b1.txt");
    TEST_ASSERT_EQUAL_INT(0, test_write_file(file_b, "b1"));

    char *dir_c = test_join_path(root, "dirC");
    TEST_ASSERT_EQUAL_INT(0, test_make_dir(dir_c));
    char *file_c = test_join_path(dir_c, "c1.txt");
    TEST_ASSERT_EQUAL_INT(0, test_write_file(file_c, "c1"));

    free(root_file1);
    free(root_file2);
    free(dir_a);
    free(file_a);
    free(dir_b);
    free(file_b);
    free(dir_c);
    free(file_c);
    return root;
}

static void cleanup_walk_fixture(char *root) {
    test_remove_tree(root);
    free(root);
}

void test_walk_depth_counts(void) {
    char *root = create_walk_fixture();
    TEST_ASSERT_EQUAL_UINT(2u, (unsigned int)count_files_with_depth(root, 0));
    TEST_ASSERT_EQUAL_UINT(4u, (unsigned int)count_files_with_depth(root, 1));
    TEST_ASSERT_EQUAL_UINT(5u, (unsigned int)count_files_with_depth(root, 2));
    TEST_ASSERT_EQUAL_UINT(5u, (unsigned int)count_files_with_depth(root, -1));
    cleanup_walk_fixture(root);
}

void test_walk_missing_root_returns_error(void) {
    const char *missing = "build/test_missing_root";
    test_remove_tree(missing);

    Config config;
    init_walk_config(&config, missing, 1);
    int saved_stderr = -1;
    TEST_ASSERT_EQUAL_INT(0, test_redirect_stderr_to_null(&saved_stderr));
    TEST_ASSERT_NOT_EQUAL(0, walk_path(&config, NULL, NULL));
    test_restore_stderr(saved_stderr);
    free_config(&config);
}

void test_walk_root_file_counts_once(void) {
    char *root = test_make_temp_dir();
    TEST_ASSERT_NOT_NULL(root);
    char *file_path = test_join_path(root, "single.txt");
    TEST_ASSERT_NOT_NULL(file_path);
    TEST_ASSERT_EQUAL_INT(0, test_write_file(file_path, "data"));

    Config config;
    init_walk_config(&config, file_path, 1);
    size_t count = 0;
    TEST_ASSERT_EQUAL_INT(0, walk_path(&config, count_files_callback, &count));
    TEST_ASSERT_EQUAL_UINT(1u, (unsigned int)count);
    free_config(&config);

    free(file_path);
    test_remove_tree(root);
    free(root);
}

void run_walk_tests(void) {
    RUN_TEST(test_walk_depth_counts);
    RUN_TEST(test_walk_missing_root_returns_error);
    RUN_TEST(test_walk_root_file_counts_once);
}
