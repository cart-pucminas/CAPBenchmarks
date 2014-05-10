/*
 * Copyright (C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * <vector/vector_create.c> - vector_create() implementation.
 */

#include <stdlib.h>
#include <vector.h>

/*
 * Creates a vector.
 */
struct vector *vector_create(int n)
{
	struct vector *v;
	
	v = malloc(sizeof(struct vector));
	
	/* Failed to allocate object. */
	if (v == NULL)
		goto error0;
	
	v->elements = calloc(n, sizeof(float));
	
	/* Failed to allocate vector. */
	if (v->elements == NULL)
		goto error1;

	/* Initilize vector. */
	v->size = n;

	return (v);

error1:
	free(v);
error0:
	return (NULL);
}
