#ifndef SCANNER_H
#define SCANNER_H

#include <stddef.h>

#include "rules.h"

/* Scanner interface for reading files and emitting findings. */

/* Scanner state shared across file scans. */
/* The scanner prints findings directly to stdout. */
typedef struct {
    /* Rules engine used to detect secrets in each line. */
    RulesEngine *rules;
    /* Total number of findings reported so far. */
    size_t finding_count;
} ScannerContext;

/* Initialize the scanner with a rules engine (rules are not owned). */
void scanner_init(ScannerContext *scanner, RulesEngine *rules);

/* Scan a file path and report findings to stdout (returns 0 on success). */
/* Returns -1 if the file cannot be opened or read. */
int scanner_scan_path(ScannerContext *scanner, const char *path);

/* Scan standard input and report findings to stdout (returns 0 on success). */
/* Returns -1 if reading from stdin fails. */
int scanner_scan_stdin(ScannerContext *scanner);

#endif /* SCANNER_H */
