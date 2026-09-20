#ifndef POOL_H
#define POOL_H

#include "queue.h"
#include <pthread.h>

typedef struct {
    int thread_count;
    pthread_t *threads;
    bounded_queue_t *queue;
    
    // Completion tracking for precise timing
    int target_jobs;
    int completed_jobs;
    pthread_mutex_t completion_mutex;
    pthread_cond_t all_completed_cond;
} thread_pool_t;

thread_pool_t* pool_create(int num_threads, int queue_capacity);
void pool_submit(thread_pool_t *pool, job_t job);
void pool_wait(thread_pool_t *pool, int expected_jobs);
void pool_shutdown(thread_pool_t *pool);

#endif