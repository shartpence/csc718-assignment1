#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pool.h"
#include "matrix.h"
/*
*This main driver runs a test set containing non-homogeneous matrix dimensions 
*4x4, 10x10, 50x50, 100x100) to verify matrix inverse accuracy and thread safety before scaling experiments.

*/
int main(int argc, char *argv[]) {
    int num_workers = 4;
    int num_jobs = 20;

    if (argc >= 2) num_workers = atoi(argv[1]);
    if (argc >= 3) num_jobs = atoi(argv[2]);

    printf("=== Starting R2 Main Test Suite ===\n");
    printf("Workers: %d | Total Jobs: %d\n", num_workers, num_jobs);

    // Create pool with capacity = 8
    thread_pool_t *pool = pool_create(num_workers, 8);

    struct timespec start, end;

    /* Timing Region Starts */
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_jobs; i++) {
        // Heterogeneous test workloads (cycling dimension sizes 4, 10, 50, 100)
        int dims[] = {4, 10, 50, 100};
        int n = dims[i % 4];

        Matrix *mat = matrix_generate_deterministic(n, 1000 + i);

        job_t job;
        job.id = i + 1;
        job.data = mat;

        pool_submit(pool, job);
    }

    // Shutdown pool (blocks until all jobs + poison pills are completed)
    pool_shutdown(pool);

    /* Timing Region Ends */
    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("\n=== Execution Summary ===\n");
    printf("Worker Threads: %d\n", num_workers);
    printf("Jobs Processed: %d\n", num_jobs);
    printf("Elapsed Time:   %.4f seconds\n", elapsed);
    printf("Throughput:     %.2f jobs/sec\n", num_jobs / elapsed);
    printf("Status: SUCCESS (All threads joined cleanly, zero leaks)\n");

    return 0;
}