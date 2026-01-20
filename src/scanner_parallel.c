#include "scanner_parallel.h"

#include <stdlib.h>
#include <unistd.h>

#include "thread_pool.h"
#include "util.h"
#include "walk.h"

#define DEFAULT_QUEUE_CAPACITY 256

typedef struct {
    ScannerContext scanner;
} WorkerContext;

static size_t get_cpu_count(void) {
    long count = sysconf(_SC_NPROCESSORS_ONLN);
    if (count < 1) {
        return 1;
    }
    return (size_t)count;
}

static size_t resolve_thread_count(int requested) {
    if (requested <= 0) {
        return get_cpu_count();
    }
    return (size_t)requested;
}

static void scan_job(void *job, void *worker_context, void *shared_context) {
    (void)shared_context;
    WorkerContext *worker = (WorkerContext *)worker_context;
    const char *path = (const char *)job;
    scanner_scan_path(&worker->scanner, path);
}

static void free_job(void *job) {
    free(job);
}

static int enqueue_path_callback(const char *path, void *user_data) {
    ThreadPool *pool = (ThreadPool *)user_data;
    char *copy = duplicate_string(path);
    if (!copy) {
        return -1;
    }
    if (thread_pool_submit(pool, copy) != 0) {
        free(copy);
        return -1;
    }
    return 0;
}

static int scan_file_callback(const char *path, void *user_data) {
    ScannerContext *scanner = (ScannerContext *)user_data;
    if (scanner_scan_path(scanner, path) != 0) {
        return 0;
    }
    return 0;
}

int scanner_scan_parallel(const Config *config, RulesEngine *rules, ScannerContext *scanner) {
    if (!config || !rules || !scanner) {
        return -1;
    }

    scanner_init(scanner, rules);

    if (config->stdin_mode) {
        return scanner_scan_stdin(scanner);
    }

    size_t thread_count = resolve_thread_count(config->threads);
    if (thread_count <= 1) {
        int walk_result = walk_path(config, scan_file_callback, scanner);
        if (walk_result != 0) {
            scanner->scan_failed = true;
            return -1;
        }
        return 0;
    }

    WorkerContext *workers = calloc(thread_count, sizeof(*workers));
    void **worker_contexts = calloc(thread_count, sizeof(*worker_contexts));
    if (!workers || !worker_contexts) {
        free(workers);
        free(worker_contexts);
        scanner->scan_failed = true;
        return -1;
    }

    for (size_t i = 0; i < thread_count; ++i) {
        scanner_init(&workers[i].scanner, rules);
        worker_contexts[i] = &workers[i];
    }

    ThreadPool *pool = thread_pool_create(thread_count,
                                          DEFAULT_QUEUE_CAPACITY,
                                          scan_job,
                                          free_job,
                                          NULL,
                                          worker_contexts);
    if (!pool) {
        free(worker_contexts);
        free(workers);
        scanner->scan_failed = true;
        return -1;
    }

    int walk_result = walk_path(config, enqueue_path_callback, pool);

    thread_pool_wait(pool);
    thread_pool_destroy(pool);

    for (size_t i = 0; i < thread_count; ++i) {
        scanner_merge(scanner, &workers[i].scanner);
        scanner_destroy(&workers[i].scanner);
    }

    free(worker_contexts);
    free(workers);

    if (walk_result != 0) {
        scanner->scan_failed = true;
        return -1;
    }

    return 0;
}
