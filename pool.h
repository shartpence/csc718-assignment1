#ifndef POOL_H
#define POOL_H

#include "queue.h"
#include <pthread.h>

typedef struct {
    int thread_count;
    pthread_t *threads;
    bounded_queue_t *queue;
} thread_pool_t;

thread_pool_t* pool_create(int num_threads, int queue_capacity);
void pool_submit(thread_pool_t *pool, job_t job);
void pool_shutdown(thread_pool_t *pool);

#endif