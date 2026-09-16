#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

typedef struct {
    int id;          // Job ID for correctness verification
    void *data;      // Payload pointer (will hold matrix structs in R2/R3)
} job_t;

typedef struct {
    job_t *buffer;
    int capacity;
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} bounded_queue_t;

bounded_queue_t* queue_init(int capacity);
void queue_destroy(bounded_queue_t *q);
void queue_enqueue(bounded_queue_t *q, job_t job);
job_t queue_dequeue(bounded_queue_t *q);

#endif