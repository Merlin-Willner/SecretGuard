#include "unity.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

void test_duplicate_string_null_returns_null(void) {
    TEST_ASSERT_NULL(duplicate_string(NULL));
}

void test_duplicate_string_copies_text(void) {
    char original[] = "hello";
    char *copy = duplicate_string(original);

    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_EQUAL_STRING(original, copy);
    TEST_ASSERT_TRUE(copy != original);

    free(copy);
}

void test_duplicate_string_empty_string(void) {
    char original[] = "";
    char *copy = duplicate_string(original);

    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_EQUAL_STRING("", copy);
    TEST_ASSERT_TRUE(copy != original);

    free(copy);
}

void run_util_tests(void) {
    RUN_TEST(test_duplicate_string_null_returns_null);
    RUN_TEST(test_duplicate_string_copies_text);
    RUN_TEST(test_duplicate_string_empty_string);
}
