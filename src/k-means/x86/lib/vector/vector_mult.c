/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * vector/vector_mult.c - vector_mult() implementation.
 */

#include <vector.h>
#include <stdlib.h>

/*
 * Multiplies a vector by a scalar.
 */
struct vector *vector_mult(struct vector *v, float scalar)
{
	int i; /* Loop index.  */
	int n; /* Vector size. */
	
	n = vector_size(v);
	
	/* Add vectors. */
	for (i = 0; i < n; i++)
		VECTOR(v, i) *= scalar;
	
	return (v);
}
