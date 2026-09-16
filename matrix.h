#ifndef MATRIX_H
#define MATRIX_H

#include <stdbool.h>

typedef struct {
    int n;           // Dimensions: n x n
    double *data;    // Continuous array (size n*n)
} Matrix;

Matrix* matrix_create(int n);
void matrix_free(Matrix *m);
Matrix* matrix_generate_deterministic(int n, unsigned int seed);
Matrix* matrix_invert(const Matrix *A);
bool matrix_verify_inverse(const Matrix *A, const Matrix *A_inv, double tolerance);

#endif