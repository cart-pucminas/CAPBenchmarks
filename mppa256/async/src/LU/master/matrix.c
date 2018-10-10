/* Kernel Includes */
#include <util.h>
#include "matrix.h"

/* C And MPPA Library Includes*/
#include <stdlib.h>
#include <stdio.h>

/* Creates a matrix. */
struct matrix *matrix_create(int height, int width) {

	if ((height <= 0) || (width <= 0))
		error("Matrix dimensions invalid");

	struct matrix *m;
	
	m = smalloc(sizeof(struct matrix));

	/* Initializing matrix. */
	m->height = height;
	m->width = width;
	m->elements = scalloc(height*width, sizeof(float));
	
	return (m);
}

/* Destroys a matrix. */
void matrix_destroy(struct matrix *m) {
	if (m == NULL)
		error("Null Matrix");
	
	free(m->elements);
	free(m);
}

/* Fills up a matrix with random numbers. */
void matrix_random(struct matrix *m) {
	if (m == NULL)
		error("Null Matrix");
	
	/* Fill matrix. */
	for (int i = 0; i < m->height; i++) {
		for (int j = 0; j < m->width; j++)
			MATRIX(m, i, j).num = randnum();
			MATRIX(m, i,j).den = 1;
	}
}

/* Copies all elements of a matrix to another matrix. */
void copyMatrix(struct matrix *m, struct matrix *m_copy) {
	for (int i = 0; i < m->height; i++) {
		for (int j = 0; j < m->width; j++) {
			MATRIX(m_copy, i, j) = MATRIX(m, i, j);
		}
	}
}

/* Multiplicates matrix l*u and stores it on matrix m. */
void matrixMult(struct matrix *l, struct matrix *u, struct matrix *m) {
	#pragma omp parallel for default(shared) num_threads(3)
    for (int i = 0; i < m->height; i++) { 
        for (int j = 0; j < m->width; j++) { 
            MATRIX(m, i, j).num = 0; 
            MATRIX(m,i,j).den = 1;
            for (int k = 0; k < l->width; k++) {
            	MATRIX(m, i, j).num += MATRIX(l, i, k) * MATRIX(u, k, j);
            	MATRIX(m, i, j).den +=
            }
        } 
    }
}

/* Compares two matrices to see their equality. */
int compareMatrices(struct matrix *m1, struct matrix *m2) {
	int resp = 1;
	#pragma omp parallel for shared(resp) default(shared) num_threads(3)
	for (int i = 0; i < m1->height; i++) {
		for (int j = 0; j < m1->width; j++) {
			if (resp) {
				if (MATRIX(m1,i,j) != MATRIX(m2,i,j)) {
					#pragma omp critical(notEqual)
					{
						if (resp)
							resp = 0;
					}
					break;
				}
			}
		}
	}
	return resp;
}