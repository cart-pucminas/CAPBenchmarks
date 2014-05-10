/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * vector/vector_sub.c - vector_sub() implementation.
 */

#include <vector.h>
#include <stdlib.h>

/*
 * Subtracts two vectors.
 */
struct vector *vector_sub(struct vector *v1,const struct vector *v2)
{
	int i; /* Loop index.  */
	int n; /* Vector size. */
	
	/* Invalid argument. */
	if (vector_size(v1) != vector_size(v2))
		return (NULL);
	
	n = vector_size(v1);
	
	/* Subtract vectors. */
	for (i = 0; i < n; i++)
		VECTOR(v1, i) -= VECTOR(v2, i);
	
	return (v1);
}
