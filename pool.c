#include "pool.h"
#include "matrix.h"
#include <stdlib.h>
#include <stdio.h>

#define POISON_PILL_ID -1

static void* worker_routine(void *arg) {
    thread_pool_t *pool = (thread_pool_t*)arg;

    while (1) {
        job_t job = queue_dequeue(pool->queue);

        if (job.id == POISON_PILL_ID) {
            break;
        }

        Matrix *A = (Matrix*)job.data;
        if (A) {
            Matrix *inv = matrix_invert(A);
            matrix_free(inv);
            matrix_free(A);
        }

        // Increment completion counter and notify main thread
        pthread_mutex_lock(&pool->completion_mutex);
        pool->completed_jobs++;
        if (pool->completed_jobs == pool->target_jobs) {
            pthread_cond_signal(&pool->all_completed_cond);
        }
        pthread_mutex_unlock(&pool->completion_mutex);
    }
    return NULL;
}

thread_pool_t* pool_create(int num_threads, int queue_capacity) {
    thread_pool_t *pool = malloc(sizeof(thread_pool_t));
    if (!pool) return NULL;

    pool->thread_count = num_threads;
    pool->queue = queue_init(queue_capacity);
    pool->threads = malloc(sizeof(pthread_t) * num_threads);

    pool->target_jobs = 0;
    pool->completed_jobs = 0;
    pthread_mutex_init(&pool->completion_mutex, NULL);
    pthread_cond_init(&pool->all_completed_cond, NULL);

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&pool->threads[i], NULL, worker_routine, pool);
    }

    return pool;
}

void pool_submit(thread_pool_t *pool, job_t job) {
    queue_enqueue(pool->queue, job);
}

void pool_wait(thread_pool_t *pool, int expected_jobs) {
    pthread_mutex_lock(&pool->completion_mutex);
    pool->target_jobs = expected_jobs;
    while (pool->completed_jobs < pool->target_jobs) {
        pthread_cond_wait(&pool->all_completed_cond, &pool->completion_mutex);
    }
    pthread_mutex_unlock(&pool->completion_mutex);
}

void pool_shutdown(thread_pool_t *pool) {
    if (!pool) return;

    for (int i = 0; i < pool->thread_count; i++) {
        job_t poison_pill = {.id = POISON_PILL_ID, .data = NULL};
        queue_enqueue(pool->queue, poison_pill);
    }

    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&pool->completion_mutex);
    pthread_cond_destroy(&pool->all_completed_cond);
    queue_destroy(pool->queue);
    free(pool->threads);
    free(pool);
}