/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <matrix.h> - Matrix library.
 */

#ifndef MATRIX_H_
#define MATRIX_H_

	#include <global.h>

	/*
	 * Matrix.
	 */
	struct matrix
	{
		int height;      /* Height.   */
		int width;       /* Width.    */
		float *elements; /* Elements. */
	};
	
	/*
	 * Opaque pointer to a matrix.
	 */
	typedef struct matrix * matrix_t;
	
	/*
	 * Caster to a matrix pointer.
	 */
	#define MATRIXP(m) \
		((matrix_t)(m))

	/*
	 * Returns the element [i][j] in a matrix.
	 */
	#define MATRIX(m, i, j)                                 \
		(MATRIXP(m)->elements[(i)*MATRIXP(m)->width + (j)]) \

	/*
	 * Creates a matrix.
	 */
	extern matrix_t matrix_create(int height, int width);
	
	/*
	 * Destroys a matrix.
	 */
	extern void matrix_destroy(matrix_t m);
	
	/*
	 * Performs LU factorization.
	 */
	extern int matrix_lu(matrix_t m, matrix_t l, matrix_t u);
	
	/*
	 * Fills up a matrix with random numbers.
	 */
	extern void matrix_random(matrix_t m);

#endif /* MATRIX_H_ */
