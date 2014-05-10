/*
 * Copyright (C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * <vector/vector_equal.c> - vector_equal() implementation.
 */

#include <vector.h>

/*
 * Tests if two vectors are equal.
 */
int vector_equal(const struct vector *a, const struct vector *b)
{
	int i;
	
	/* Vectors differ on size. */
	if (a->size != b->size)
		return (0);
	
	/* Test all elements. */
	for (i = 0; i < a->size; i++)
	{
		if (VECTOR(a, i) != VECTOR(b, i))
			return (0);
	}
	
	return (1);
}

