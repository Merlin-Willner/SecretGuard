#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <stddef.h>

typedef void (*thread_job_fn)(void *job, void *worker_context, void *shared_context);
typedef void (*thread_job_cleanup_fn)(void *job);

typedef struct ThreadPool ThreadPool;

ThreadPool *thread_pool_create(size_t thread_count,
                               size_t queue_capacity,
                               thread_job_fn job_fn,
                               thread_job_cleanup_fn cleanup_fn,
                               void *shared_context,
                               void **worker_contexts);

int thread_pool_submit(ThreadPool *pool, void *job);
void thread_pool_stop(ThreadPool *pool);
void thread_pool_destroy(ThreadPool *pool);

#endif /* THREAD_POOL_H */
