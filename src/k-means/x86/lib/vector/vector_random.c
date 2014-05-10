/*
 * Copyright(C) 2011-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * vector/vector_random.c - vector_random() implementaiton.
 */
 
#include <stdlib.h>
#include <vector.h>
#include <globl.h>
#include "random.h"

/* extern int myrand(void); */
extern inline unsigned int simple_rng_rand();

/*
 * Creates a random vector.
 */
struct vector *vector_random(int n)
{
	int i;            /* Loop index. */
	struct vector *v; /* Vector.     */
	
	v = malloc(sizeof(struct vector));
	
	/* Failed to allocate object. */
	if (v == NULL)
		goto error0;
	
	v->elements = malloc(n*sizeof(float));
	
	/* Failed to allocate vector. */
	if (v->elements == NULL)
		goto error1;

	/* Generate random vector. */
	for (i = 0; i < n; i++)
	  VECTOR(v, i) = simple_rng_rand() % n;
	v->size = n;

	return (v);

error1:
	free(v);
error0:
	return (NULL);
}
