#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <stddef.h>

// Called by a worker thread to process a job.
typedef void (*thread_job_fn)(void *job, void *worker_context, void *shared_context);
// Optional cleanup for a job after it was processed.
typedef void (*thread_job_cleanup_fn)(void *job);

typedef struct ThreadPool ThreadPool;

// Create a thread pool with a bounded queue.
// worker_contexts can be NULL or one pointer per worker.
ThreadPool *thread_pool_create(size_t thread_count,
                               size_t queue_capacity,
                               thread_job_fn job_fn,
                               thread_job_cleanup_fn cleanup_fn,
                               void *shared_context,
                               void **worker_contexts);

// Submit a job to the pool (blocks if the queue is full).
int thread_pool_submit(ThreadPool *pool, void *job);

// Wait until all queued jobs are finished.
void thread_pool_wait(ThreadPool *pool);

// Signal workers to stop after current work.
void thread_pool_stop(ThreadPool *pool);

// Stop the pool and free its resources.
void thread_pool_destroy(ThreadPool *pool);

#endif /* THREAD_POOL_H */
