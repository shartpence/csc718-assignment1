#include "pool.h"
#include "matrix.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * POISON PILL SHUTDOWN MECHANISM:
 * A job with id = -1 signals worker threads to terminate.
 */
#define POISON_PILL_ID -1

static void* worker_routine(void *arg) {
    bounded_queue_t *queue = (bounded_queue_t*)arg;

    while (1) {
        /*
         * VULNERABILITY PREVENTION (Deadlock / Lost Signals):
         * queue_dequeue() encapsulates the mutex lock/unlock and
         * condition variable wait. Threads will block safely here
         * without consuming CPU cycles when the queue is empty.
         */
        job_t job = queue_dequeue(queue);

        /* Poison Pill Check */
        if (job.id == POISON_PILL_ID) {
            break; // Exit worker loop cleanly
        }

        /* Execute Matrix Inversion Task */
        Matrix *A = (Matrix*)job.data;
        if (A) {
            Matrix *inv = matrix_invert(A);
            
            // Verify correctness for small/test inputs
            bool ok = matrix_verify_inverse(A, inv, 1e-4);
            if (!ok) {
                fprintf(stderr, "[ERROR] Matrix verification failed for Job ID %d\n", job.id);
            }

            // Prevent memory leaks: worker frees job payloads once executed
            matrix_free(inv);
            matrix_free(A);
        }
    }
    return NULL;
}

thread_pool_t* pool_create(int num_threads, int queue_capacity) {
    thread_pool_t *pool = malloc(sizeof(thread_pool_t));
    if (!pool) return NULL;

    pool->thread_count = num_threads;
    pool->queue = queue_init(queue_capacity);
    pool->threads = malloc(sizeof(pthread_t) * num_threads);

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&pool->threads[i], NULL, worker_routine, pool->queue);
    }

    return pool;
}

void pool_submit(thread_pool_t *pool, job_t job) {
    queue_enqueue(pool->queue, job);
}

void pool_shutdown(thread_pool_t *pool) {
    if (!pool) return;

    /*
     * VULNERABILITY PREVENTION (Thread Leak / Deadlock during Shutdown):
     * Submit exactly N poison pills so every worker thread wakes up, 
     * exits its loop, and finishes execution cleanly.
     */
    for (int i = 0; i < pool->thread_count; i++) {
        job_t poison_pill = {.id = POISON_PILL_ID, .data = NULL};
        queue_enqueue(pool->queue, poison_pill);
    }

    /* Wait for all worker threads to terminate */
    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    /* Destroy queue and free pool memory */
    queue_destroy(pool->queue);
    free(pool->threads);
    free(pool);
}