/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * vector/vector_add.c - vector_add() implementation.
 */

#include <vector.h>
#include <stdlib.h>

/*
 * Adds two vectors.
 */
struct vector *vector_add(struct vector *v1, const struct vector *v2)
{
	int i; /* Loop index.  */
	int n; /* Vector size. */
	
	/* Invalid argument. */
	if (vector_size(v1) != vector_size(v2))
		return (NULL);
	
	n = vector_size(v1);
	
	/* Add vectors. */
	for (i = 0; i < n; i++)
		VECTOR(v1, i) += VECTOR(v2, i);
	
	return (v1);
}
