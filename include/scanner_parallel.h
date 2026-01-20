#ifndef SCANNER_PARALLEL_H
#define SCANNER_PARALLEL_H

#include "config.h"
#include "rules.h"
#include "scanner.h"

// Scan with optional parallelism based on config->threads.
// Returns 0 on success, -1 on error.
int scanner_scan_parallel(const Config *config, RulesEngine *rules, ScannerContext *scanner);

#endif /* SCANNER_PARALLEL_H */
