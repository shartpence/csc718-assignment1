#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"

#define NUM_PRODUCERS 4
#define NUM_CONSUMERS 4
#define JOBS_PER_PRODUCER 1000
#define QUEUE_CAPACITY 8

bounded_queue_t *q;
long long total_consumed_sum = 0;
pthread_mutex_t sum_mutex = PTHREAD_MUTEX_INITIALIZER;

void* producer(void *arg) {
    int producer_id = *(int*)arg;
    for (int i = 1; i <= JOBS_PER_PRODUCER; i++) {
        job_t job;
        job.id = producer_id * 10000 + i;
        job.data = NULL;
        queue_enqueue(q, job);
    }
    return NULL;
}

void* consumer(void *arg) {
    (void)arg;
    int jobs_to_consume = (NUM_PRODUCERS * JOBS_PER_PRODUCER) / NUM_CONSUMERS;
    for (int i = 0; i < jobs_to_consume; i++) {
        job_t job = queue_dequeue(q);

        pthread_mutex_lock(&sum_mutex);
        total_consumed_sum += job.id;
        pthread_mutex_unlock(&sum_mutex);
    }
    return NULL;
}

int main(void) {
    q = queue_init(QUEUE_CAPACITY);

    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int producer_ids[NUM_PRODUCERS];

    long long expected_sum = 0;
    for (int p = 0; p < NUM_PRODUCERS; p++) {
        producer_ids[p] = p + 1;
        for (int i = 1; i <= JOBS_PER_PRODUCER; i++) {
            expected_sum += (p + 1) * 10000 + i;
        }
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) pthread_create(&consumers[i], NULL, consumer, NULL);
    for (int i = 0; i < NUM_PRODUCERS; i++) pthread_create(&producers[i], NULL, producer, &producer_ids[i]);

    for (int i = 0; i < NUM_PRODUCERS; i++) pthread_join(producers[i], NULL);
    for (int i = 0; i < NUM_CONSUMERS; i++) pthread_join(consumers[i], NULL);

    printf("R1 Test Results:\n");
    printf("Expected Sum: %lld\n", expected_sum);
    printf("Observed Sum: %lld\n", total_consumed_sum);

    if (expected_sum == total_consumed_sum) {
        printf("VERIFICATION PASSED: All jobs processed exactly once.\n");
    } else {
        printf("VERIFICATION FAILED: Sum mismatch detected.\n");
    }

    queue_destroy(q);
    pthread_mutex_destroy(&sum_mutex);
    return 0;
}
//compile instructions
//gcc -Wall -Wextra -pthread -O2 queue.c r1_test.c -o r1_test
//./r1_test

//if needed, can check fo memor leaks or condition varialbe bugs with:
//valgrind --tool=helgrind ./r1_test