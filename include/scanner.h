#ifndef SCANNER_H
#define SCANNER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include "rules.h"

/* Scanner interface for reading files and emitting findings. */

/* Scanner state shared across file scans. */
/* Findings are stored in a linked list for reporting after scanning. */
typedef struct ScannerFindingNode ScannerFindingNode;

typedef struct {
    /* Rules engine used to detect secrets in each line. */
    RulesEngine *rules;
    /* Total number of findings reported so far. */
    size_t finding_count;
    /* Highest severity seen so far (valid only if finding_count > 0). */
    severity_t highest_severity;
    /* File scan counters. */
    size_t files_scanned;
    size_t files_skipped;
    /* Linked list of findings for reporting after scanning. */
    ScannerFindingNode *findings_head;
    ScannerFindingNode *findings_tail;
} ScannerContext;

/* Initialize the scanner with a rules engine (rules are not owned). */
void scanner_init(ScannerContext *scanner, RulesEngine *rules);

/* Print summary and results after scanning. */
void scanner_print_report(const ScannerContext *scanner, FILE *out);

/* Print JSON summary and results after scanning. */
void scanner_print_report_json(const ScannerContext *scanner, FILE *out);

/* Release memory allocated for stored findings. */
void scanner_destroy(ScannerContext *scanner);

/* Scan a file path and report findings to stdout (returns 0 on success). */
/* Returns -1 if the file cannot be opened or read. */
int scanner_scan_path(ScannerContext *scanner, const char *path);

/* Scan standard input and report findings to stdout (returns 0 on success). */
/* Returns -1 if reading from stdin fails. */
int scanner_scan_stdin(ScannerContext *scanner);

#endif /* SCANNER_H */
