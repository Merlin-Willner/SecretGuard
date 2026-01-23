#ifndef SCANNER_H
#define SCANNER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include "rules.h"

typedef struct ScannerFindingNode ScannerFindingNode;

typedef struct {
    RulesEngine *rules;
    size_t finding_count;
    severity_t highest_severity;
    size_t files_scanned;
    size_t files_skipped;
    bool scan_failed;
    ScannerFindingNode *findings_head;
    ScannerFindingNode *findings_tail;
} ScannerContext;

// Initialize the scanner with a rules engine (rules are not owned).
void scanner_init(ScannerContext *scanner, RulesEngine *rules);

// Print a human-readable report to out (stdout if NULL).
void scanner_print_report(const ScannerContext *scanner, FILE *out);

// Print a JSON report to out (stdout if NULL).
void scanner_print_report_json(const ScannerContext *scanner, FILE *out);

// Free memory used by the stored findings.
void scanner_destroy(ScannerContext *scanner);

// Merge results from src into dest.
void scanner_merge(ScannerContext *dest, ScannerContext *src);

// Scan one file path. Returns 0 on success, -1 on error.
int scanner_scan_path(ScannerContext *scanner, const char *path);

// Scan standard input. Returns 0 on success, -1 on error.
int scanner_scan_stdin(ScannerContext *scanner);

#endif /* SCANNER_H */
