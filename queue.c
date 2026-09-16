#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

bounded_queue_t* queue_init(int capacity) {
    bounded_queue_t *q = malloc(sizeof(bounded_queue_t));
    if (!q) return NULL;

    q->buffer = malloc(sizeof(job_t) * capacity);
    if (!q->buffer) {
        free(q);
        return NULL;
    }

    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;
    q->count = 0;

    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->not_full, NULL);
    pthread_cond_init(&q->not_empty, NULL);

    return q;
}

void queue_destroy(bounded_queue_t *q) {
    if (!q) return;
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->not_full);
    pthread_cond_destroy(&q->not_empty);
    free(q->buffer);
    free(q);
}

void queue_enqueue(bounded_queue_t *q, job_t job) {
    pthread_mutex_lock(&q->mutex);

    // Block while full; while loop prevents spurious wakeups
    while (q->count == q->capacity) {
        
        pthread_cond_wait(&q->not_full, &q->mutex);
    }

    q->buffer[q->tail] = job;
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;

    // Signal waiting consumers that items are available
    pthread_cond_signal(&q->not_empty);

    pthread_mutex_unlock(&q->mutex);
}

job_t queue_dequeue(bounded_queue_t *q) {
    pthread_mutex_lock(&q->mutex);

    // Block while empty
    while (q->count == 0) {
        
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }

    job_t job = q->buffer[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->count--;

    // Signal waiting producers that space is available
    pthread_cond_signal(&q->not_full);

    pthread_mutex_unlock(&q->mutex);

    return job;
}