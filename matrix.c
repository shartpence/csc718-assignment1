#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

Matrix* matrix_create(int n) {
    Matrix *m = malloc(sizeof(Matrix));
    if (!m) return NULL;
    m->n = n;
    m->data = calloc(n * n, sizeof(double));
    if (!m->data) {
        free(m);
        return NULL;
    }
    return m;
}

void matrix_free(Matrix *m) {
    if (!m) return;
    free(m->data);
    free(m);
}

Matrix* matrix_generate_deterministic(int n, unsigned int seed) {
    Matrix *m = matrix_create(n);
    if (!m) return NULL;

    srand(seed);
    for (int i = 0; i < n * n; i++) {
        // Values between 1.0 and 10.0
        m->data[i] = 1.0 + ((double)rand() / (double)RAND_MAX) * 9.0;
        if (i / n == i % n) {
            // Strictly diagonally dominant to ensure invertibility
            m->data[i] += n * 10.0;
        }
    }
    return m;
}

Matrix* matrix_invert(const Matrix *A) {
    int n = A->n;
    // Create augmented matrix [A | I] of size n x (2n)
    double *aug = malloc(n * 2 * n * sizeof(double));
    if (!aug) return NULL;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            aug[i * (2 * n) + j] = A->data[i * n + j];
            aug[i * (2 * n) + (j + n)] = (i == j) ? 1.0 : 0.0;
        }
    }

    // Gauss-Jordan elimination with partial pivoting
    for (int i = 0; i < n; i++) {
        // Find pivot
        int pivot = i;
        for (int k = i + 1; k < n; k++) {
            if (fabs(aug[k * (2 * n) + i]) > fabs(aug[pivot * (2 * n) + i])) {
                pivot = k;
            }
        }

        // Swap pivot row
        if (pivot != i) {
            for (int j = 0; j < 2 * n; j++) {
                double temp = aug[i * (2 * n) + j];
                aug[i * (2 * n) + j] = aug[pivot * (2 * n) + j];
                aug[pivot * (2 * n) + j] = temp;
            }
        }

        double pivot_val = aug[i * (2 * n) + i];
        if (fabs(pivot_val) < 1e-12) { // Singular matrix check
            free(aug);
            return NULL; 
        }

        // Normalize row i
        for (int j = 0; j < 2 * n; j++) {
            aug[i * (2 * n) + j] /= pivot_val;
        }

        // Eliminate column i in all other rows
        for (int k = 0; k < n; k++) {
            if (k != i) {
                double factor = aug[k * (2 * n) + i];
                for (int j = 0; j < 2 * n; j++) {
                    aug[k * (2 * n) + j] -= factor * aug[i * (2 * n) + j];
                }
            }
        }
    }

    // Extract inverse matrix
    Matrix *inv = matrix_create(n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            inv->data[i * n + j] = aug[i * (2 * n) + (j + n)];
        }
    }

    free(aug);
    return inv;
}

bool matrix_verify_inverse(const Matrix *A, const Matrix *A_inv, double tolerance) {
    if (!A || !A_inv || A->n != A_inv->n) return false;
    int n = A->n;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A->data[i * n + k] * A_inv->data[k * n + j];
            }
            double expected = (i == j) ? 1.0 : 0.0;
            if (fabs(sum - expected) > tolerance) {
                return false;
            }
        }
    }
    return true;
}