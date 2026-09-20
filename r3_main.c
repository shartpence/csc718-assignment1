#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pool.h"
#include "matrix.h"

#define MATRIX_DIM 250   // 250x250 matrix inversion (~0.015s per job)
#define QUEUE_CAP 16

int main(int argc, char *argv[]) {
    int num_workers = 1;
    int num_jobs = 200;  // 200 jobs -> ~3 to 4 seconds baseline for N=1

    if (argc >= 2) num_workers = atoi(argv[1]);
    if (argc >= 3) num_jobs = atoi(argv[2]);

    printf("=== R3 Scaling Experiment ===\n");
    printf("Workers (N): %d | Jobs (M): %d | Workload: %dx%d matrices\n", 
           num_workers, num_jobs, MATRIX_DIM, MATRIX_DIM);

    // 1. Create worker pool (threads created here)
    thread_pool_t *pool = pool_create(num_workers, QUEUE_CAP);

    struct timespec start, end;

    // 2. START TIMER (after threads created, before job submission)
    clock_gettime(CLOCK_MONOTONIC, &start);

    // 3. Submit deterministic jobs
    for (int i = 0; i < num_jobs; i++) {
        Matrix *mat = matrix_generate_deterministic(MATRIX_DIM, 1000 + i);
        job_t job = {.id = i + 1, .data = mat};
        pool_submit(pool, job);
    }

    // 4. Wait for all M jobs to complete
    pool_wait(pool, num_jobs);

    // 5. STOP TIMER (after jobs complete, before pool shutdown)
    clock_gettime(CLOCK_MONOTONIC, &end);

    // 6. Shutdown workers
    pool_shutdown(pool);

    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Wall-Clock Time: %.4f seconds\n", elapsed);
    printf("Throughput:      %.2f jobs/sec\n", num_jobs / elapsed);

    return 0;
}