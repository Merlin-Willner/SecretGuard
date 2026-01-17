#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

void run_scanner_tests(void);
void run_cli_tests(void);
void run_walk_tests(void);
void run_rules_tests(void);
void run_config_tests(void);
void run_util_tests(void);
void run_app_tests(void);

int main(void) {
    UNITY_BEGIN();
    run_util_tests();
    run_config_tests();
    run_scanner_tests();
    run_cli_tests();
    run_walk_tests();
    run_rules_tests();
    run_app_tests();
    return UNITY_END();
}
