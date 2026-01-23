// Simple thread pool with a bounded queue.

#include "thread_pool.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    struct ThreadPool *pool;
    size_t index;
} WorkerArgs;

struct ThreadPool {
    pthread_t *threads;
    size_t thread_count;
    WorkerArgs *worker_args;

    void **queue;
    size_t queue_capacity;
    size_t queue_head;
    size_t queue_tail;
    // Number of queued or running jobs.
    size_t pending_jobs;

    pthread_mutex_t mutex;
    pthread_cond_t idle_cond;
    sem_t jobs_available;
    sem_t slots_available;
    bool shutdown;

    thread_job_fn job_fn;
    thread_job_cleanup_fn cleanup_fn;
    void *shared_context;
    void **worker_contexts;
};

// Worker loop: wait for jobs and process them.
static void *worker_main(void *arg) {
    WorkerArgs *args = (WorkerArgs *)arg;
    ThreadPool *pool = args->pool;
    void *worker_ctx = pool->worker_contexts ? pool->worker_contexts[args->index] : NULL;

    while (true) {
        sem_wait(&pool->jobs_available);

        pthread_mutex_lock(&pool->mutex);
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        void *job = pool->queue[pool->queue_head];
        pool->queue_head = (pool->queue_head + 1) % pool->queue_capacity;
        pthread_mutex_unlock(&pool->mutex);

        sem_post(&pool->slots_available);

        pool->job_fn(job, worker_ctx, pool->shared_context);
        if (pool->cleanup_fn) pool->cleanup_fn(job);

        pthread_mutex_lock(&pool->mutex);
        if (pool->pending_jobs > 0) {
            pool->pending_jobs--;
            if (pool->pending_jobs == 0) {
                pthread_cond_broadcast(&pool->idle_cond);
            }
        }
        pthread_mutex_unlock(&pool->mutex);
    }
    return NULL;
}

ThreadPool *thread_pool_create(size_t thread_count, size_t queue_capacity,
                               thread_job_fn job_fn, thread_job_cleanup_fn cleanup_fn,
                               void *shared_context, void **worker_contexts) {
    if (thread_count == 0 || queue_capacity == 0 || !job_fn) return NULL;

    ThreadPool *pool = calloc(1, sizeof(*pool));
    if (!pool) return NULL;

    pool->thread_count = thread_count;
    pool->queue_capacity = queue_capacity;
    pool->job_fn = job_fn;
    pool->cleanup_fn = cleanup_fn;
    pool->shared_context = shared_context;
    pool->worker_contexts = worker_contexts;

    pool->threads = calloc(thread_count, sizeof(*pool->threads));
    pool->queue = calloc(queue_capacity, sizeof(*pool->queue));
    pool->worker_args = calloc(thread_count, sizeof(*pool->worker_args));
    if (!pool->threads || !pool->queue || !pool->worker_args) goto fail;

    if (pthread_mutex_init(&pool->mutex, NULL) != 0) goto fail;
    if (pthread_cond_init(&pool->idle_cond, NULL) != 0) goto fail_mutex;
    if (sem_init(&pool->jobs_available, 0, 0) != 0) goto fail_cond;
    if (sem_init(&pool->slots_available, 0, (unsigned)queue_capacity) != 0) goto fail_sem1;

    for (size_t i = 0; i < thread_count; ++i) {
        pool->worker_args[i].pool = pool;
        pool->worker_args[i].index = i;
        if (pthread_create(&pool->threads[i], NULL, worker_main, &pool->worker_args[i]) != 0) {
            pool->shutdown = true;
            for (size_t j = 0; j < i; ++j) sem_post(&pool->jobs_available);
            for (size_t j = 0; j < i; ++j) pthread_join(pool->threads[j], NULL);
            goto fail_sem2;
        }
    }
    return pool;

fail_sem2: sem_destroy(&pool->slots_available);
fail_sem1: sem_destroy(&pool->jobs_available);
fail_cond: pthread_cond_destroy(&pool->idle_cond);
fail_mutex: pthread_mutex_destroy(&pool->mutex);
fail:
    free(pool->threads);
    free(pool->queue);
    free(pool->worker_args);
    free(pool);
    return NULL;
}

int thread_pool_submit(ThreadPool *pool, void *job) {
    if (!pool || !job) return -1;

    sem_wait(&pool->slots_available);

    pthread_mutex_lock(&pool->mutex);
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->mutex);
        sem_post(&pool->slots_available);
        return -1;
    }
    pool->queue[pool->queue_tail] = job;
    pool->queue_tail = (pool->queue_tail + 1) % pool->queue_capacity;
    pool->pending_jobs++;
    pthread_mutex_unlock(&pool->mutex);

    sem_post(&pool->jobs_available);
    return 0;
}

void thread_pool_wait(ThreadPool *pool) {
    if (!pool) return;

    pthread_mutex_lock(&pool->mutex);
    while (pool->pending_jobs > 0) {
        pthread_cond_wait(&pool->idle_cond, &pool->mutex);
    }
    pthread_mutex_unlock(&pool->mutex);
}

void thread_pool_stop(ThreadPool *pool) {
    if (!pool) return;
    pthread_mutex_lock(&pool->mutex);
    pool->shutdown = true;
    pthread_mutex_unlock(&pool->mutex);
    for (size_t i = 0; i < pool->thread_count; ++i)
        sem_post(&pool->jobs_available);
}

void thread_pool_destroy(ThreadPool *pool) {
    if (!pool) return;
    thread_pool_stop(pool);
    for (size_t i = 0; i < pool->thread_count; ++i)
        pthread_join(pool->threads[i], NULL);
    sem_destroy(&pool->slots_available);
    sem_destroy(&pool->jobs_available);
    pthread_cond_destroy(&pool->idle_cond);
    pthread_mutex_destroy(&pool->mutex);
    free(pool->threads);
    free(pool->queue);
    free(pool->worker_args);
    free(pool);
}
