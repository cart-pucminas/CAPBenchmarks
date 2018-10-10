/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef MATRIX_H_
#define MATRIX_H_

/* Matrix. */
struct matrix {
		int height;      /* Height.   */
		int width;       /* Width.    */
		float *elements; /* Elements. */
};

/* Opaque pointer to a matrix. */
typedef struct matrix * matrix_t;

/* Caster to a matrix pointer. */
#define MATRIXP(m) \
((matrix_t)(m))

/* Returns the element [i][j] in a matrix. */
#define MATRIX(m, i, j)                             \
(MATRIXP(m)->elements[(i)*MATRIXP(m)->width + (j)]) \

/* Creates a matrix. */
extern matrix_t matrix_create(int height, int width);

/* Destroys a matrix. */
extern void matrix_destroy(matrix_t m);

/* Fills up a matrix with random numbers. */
extern void matrix_random(matrix_t m);

/* Performs LU factorization. */
extern int matrix_lu(matrix_t m, matrix_t l, matrix_t u);

/* Applies the row reduction algorithm in a matrix. */
extern void row_reduction(struct matrix *m, int i0);

#endif /* MATRIX_H_ */
