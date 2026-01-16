

/* Reads every file or standard in stdin line by line
runs the regex rules on each line, and stores findings for later reporting. */



/* After that the code should work for APIs and Passwords.#
 The next step would be:
 1. Expand for more regex rules
 to detect more critical security flaws
 2. Implement multi threading for performance gains

 */

#include "scanner.h"
#include <errno.h>
#include <fcntl.h>   /* POSIX open for file descriptors */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  /* POSIX read/close for streaming file data */

#include "config.h"

/* Buffer size for reading files or lines */
#define SCAN_BUFFER_SIZE 8192

typedef struct ScannerFindingNode {
    char *rule_name;
    severity_t severity;
    char *path;
    size_t line_number;
    size_t column;
    struct ScannerFindingNode *next;
} ScannerFindingNode;

/* Structure to hold context information for a single line,
   passed to the callback function */
typedef struct {
    ScannerContext *scanner;
    const char *path;
    size_t line_number;
} LineContext;

/* Check whether a buffer looks like binary data. */
bool is_binary_buffer(const unsigned char *buffer, size_t length) {
    if (!buffer || length == 0) {
        return false;
    }

    size_t non_printable = 0;
    for (size_t i = 0; i < length; ++i) {
        unsigned char current = buffer[i];
        if (current == '\0') {
            return true;
        }
        if (current < 0x09 || (current > 0x0D && current < 0x20)) {
            non_printable++;
        }
    }

    return (double)non_printable / (double)length > 0.3;
}

/* Initialize the scanner with a rules engine, which defines
   patterns that indicate "sensitive information". */
void scanner_init(ScannerContext *scanner, RulesEngine *rules) {
    scanner->rules = rules;
    scanner->finding_count = 0; /* Reset number of findings */
    scanner->highest_severity = SEVERITY_LOW;
    scanner->findings_head = NULL;
    scanner->findings_tail = NULL;
    scanner->files_scanned = 0;
    scanner->files_skipped = 0;
}

static char *duplicate_string(const char *text) {
    if (!text) {
        return NULL;
    }
    size_t length = strlen(text) + 1;
    char *copy = malloc(length);
    if (copy) {
        memcpy(copy, text, length);
    }
    return copy;
}

static int compare_findings(const ScannerFindingNode *a, const ScannerFindingNode *b) {
    if (a->severity != b->severity) {
        return (int)b->severity - (int)a->severity;
    }

    int path_cmp = strcmp(a->path, b->path);
    if (path_cmp != 0) {
        return path_cmp;
    }

    if (a->line_number != b->line_number) {
        return (a->line_number < b->line_number) ? -1 : 1;
    }

    if (a->column != b->column) {
        return (a->column < b->column) ? -1 : 1;
    }

    return strcmp(a->rule_name, b->rule_name);
}

static int append_finding(ScannerContext *scanner,
                          const char *rule_name,
                          severity_t severity,
                          const char *path,
                          size_t line_number,
                          size_t column) {
    if (!scanner) {
        return -1;
    }

    ScannerFindingNode *node = calloc(1, sizeof(*node));
    if (!node) {
        return -1;
    }

    node->rule_name = duplicate_string(rule_name);
    node->path = duplicate_string(path);
    if (!node->rule_name || !node->path) {
        free(node->rule_name);
        free(node->path);
        free(node);
        return -1;
    }

    node->severity = severity;
    node->line_number = line_number;
    node->column = column;

    if (!scanner->findings_head) {
        scanner->findings_head = node;
        scanner->findings_tail = node;
    } else {
        ScannerFindingNode *prev = NULL;
        ScannerFindingNode *current = scanner->findings_head;
        while (current && compare_findings(current, node) <= 0) {
            prev = current;
            current = current->next;
        }

        if (!prev) {
            node->next = scanner->findings_head;
            scanner->findings_head = node;
        } else {
            node->next = current;
            prev->next = node;
        }

        if (!node->next) {
            scanner->findings_tail = node;
        }
    }

    scanner->finding_count++;
    if (severity > scanner->highest_severity) {
        scanner->highest_severity = severity;
    }

    return 0;
}


static const char *severity_label(severity_t severity) {
    switch (severity) {
    case SEVERITY_HIGH:
        return "HIGH";
    case SEVERITY_MEDIUM:
        return "MEDIUM";
    case SEVERITY_LOW:
    default:
        return "LOW";
    }
}

static const char *severity_color(severity_t severity) {
    switch (severity) {
    case SEVERITY_HIGH:
        return "\x1b[31m";
    case SEVERITY_MEDIUM:
        return "\x1b[33m";
    case SEVERITY_LOW:
    default:
        return "\x1b[36m";
    }
}

/* Print a single finding to stdout */
static void print_finding(const char *rule_name,
                          severity_t severity,
                          const char *path,
                          size_t line_number,
                          size_t column) {
    const char *label = severity_label(severity);
    const char *color = severity_color(severity);
    const char *reset = "\x1b[0m";
    printf("%s[%s]%s %s\n", color, label, reset, rule_name);
    printf("  file: %s:%zu:%zu\n", path, line_number, column);
    printf("  Line: %zu, Col: %zu\n", line_number, column);
}

void scanner_print_report(const ScannerContext *scanner) {
    if (!scanner) {
        return;
    }

    const char *status = "OK";
    const char *status_icon = "\u2713";
    const char *status_color = "\x1b[32m";
    const char *status_reset = "\x1b[0m";
    if (scanner->finding_count > 0) {
        if (scanner->highest_severity == SEVERITY_HIGH) {
            status = "ERROR";
            status_icon = "\u2716";
            status_color = "\x1b[31m";
        } else if (scanner->highest_severity == SEVERITY_MEDIUM) {
            status = "WARN";
            status_icon = "\u26a0";
            status_color = "\x1b[33m";
        }
    }

    printf("Summary: %s%s %s%s - %zu findings | files: %zu scanned, %zu skipped\n",
           status_color,
           status_icon,
           status,
           status_reset,
           scanner->finding_count,
           scanner->files_scanned,
           scanner->files_skipped);
    printf("Results:\n");
    if (scanner->finding_count == 0) {
        printf("  (no findings)\n");
    } else {
        const ScannerFindingNode *current = scanner->findings_head;
        while (current) {
            print_finding(current->rule_name,
                          current->severity,
                          current->path,
                          current->line_number,
                          current->column);
            current = current->next;
        }
    }
}

void scanner_destroy(ScannerContext *scanner) {
    if (!scanner) {
        return;
    }

    ScannerFindingNode *current = scanner->findings_head;
    while (current) {
        ScannerFindingNode *next = current->next;
        free(current->rule_name);
        free(current->path);
        free(current);
        current = next;
    }

    scanner->findings_head = NULL;
    scanner->findings_tail = NULL;
    scanner->finding_count = 0;
    scanner->highest_severity = SEVERITY_LOW;
    scanner->files_scanned = 0;
    scanner->files_skipped = 0;
}

/* Callback function called by the rules engine when a match is found */
static void match_callback(const char *rule_name,
                           severity_t severity,
                           size_t start,
                           size_t end,
                           void *user_data) {
    (void)end;
    LineContext *line_context = (LineContext *)user_data;
    size_t column = start + 1;
    if (append_finding(line_context->scanner,
                       rule_name,
                       severity,
                       line_context->path,
                       line_context->line_number,
                       column) != 0) {
        fprintf(stderr, "ERROR: out of memory while storing findings.\n");
    }
}

/* Scan a single line for sensitive information */
static void scan_line(ScannerContext *scanner,
                      const char *path,
                      const char *line,
                      size_t length,
                      size_t line_number) {
    LineContext line_context;
    line_context.scanner = scanner;
    line_context.path = path;
    line_context.line_number = line_number;

    /* Execute the rules engine on this line */
    rules_scan_line(scanner->rules, line, length, match_callback, &line_context);
}

/* Scan an open file descriptor for secrets. */
static int scan_file_descriptor(ScannerContext *scanner,
                                const char *path,
                                int file_descriptor) {
    char buffer[SCAN_BUFFER_SIZE];
    char *line_buffer = NULL;
    size_t line_capacity = 0;
    size_t line_length = 0;
    size_t line_number = 1;
    int result = 0;
    bool checked_binary = false;

    ssize_t bytes_read = 0;
    while ((bytes_read = read(file_descriptor, buffer, sizeof(buffer))) > 0) {
        if (!checked_binary) {
            checked_binary = true;
            if (is_binary_buffer((const unsigned char *)buffer, (size_t)bytes_read)) {
                result = 1;
                goto cleanup;
            }
        }

        for (ssize_t i = 0; i < bytes_read; ++i) {
            char current = buffer[i];
            if (line_length + 2 > line_capacity) {
                size_t new_capacity = line_capacity ? line_capacity * 2 : 256;
                while (new_capacity < line_length + 2) {
                    new_capacity *= 2;
                }
                char *resized = realloc(line_buffer, new_capacity);
                if (!resized) {
                    result = -1;
                    goto cleanup;
                }
                line_buffer = resized;
                line_capacity = new_capacity;
            }

            /* Detect end of line */
            if (current == '\n') {
                if (line_length > 0 && line_buffer[line_length - 1] == '\r') {
                    line_length--;
                }
                line_buffer[line_length] = '\0';
                scan_line(scanner, path, line_buffer, line_length, line_number);
                line_length = 0;
                line_number++;
            } else {
                line_buffer[line_length] = current;
                line_length++;
            }
        }
    }

    if (bytes_read < 0) {
        fprintf(stderr, "ERROR: read failed on %s: %s\n", path, strerror(errno));
        result = -1;
    }

    /* Check the last line if the file doesn't end with \n */
    if (line_length > 0) {
        line_buffer[line_length] = '\0';
        scan_line(scanner, path, line_buffer, line_length, line_number);
    }

cleanup:
    free(line_buffer);
    return result;
}


/* Scan a file path for secret patterns. */
int scanner_scan_path(ScannerContext *scanner, const char *path) {
    if (!scanner || !path) return -1;

    int file_descriptor = open(path, O_RDONLY);
    if (file_descriptor < 0) {
        fprintf(stderr, "ERROR: failed to open %s: %s\n", path, strerror(errno));
        scanner->files_skipped++;
        return -1;
    }

    int result = scan_file_descriptor(scanner, path, file_descriptor);
    close(file_descriptor);
    if (result == 0) {
        scanner->files_scanned++;
    } else if (result > 0) {
        scanner->files_skipped++;
        result = 0;
    } else {
        scanner->files_skipped++;
    }
    return result;
}

/* Scan standard input for secret patterns. */
int scanner_scan_stdin(ScannerContext *scanner) {
    if (!scanner) return -1;

    int result = scan_file_descriptor(scanner, DEFAULT_STDIN_LABEL, STDIN_FILENO);
    if (result == 0) {
        scanner->files_scanned++;
    } else if (result > 0) {
        scanner->files_skipped++;
        result = 0;
    } else {
        scanner->files_skipped++;
    }
    return result;
}
