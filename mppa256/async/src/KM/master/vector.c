/* Kernel Includes */
#include <util.h>
#include "vector.h"

/* C And MPPA Library Includes*/
#include <math.h>
#include <string.h>

/* Creates a vector. */
struct vector *vector_create(int n) {
	struct vector *v;
	
	v = smalloc(sizeof(struct vector));
	
	/* Initilize vector. */
	v->size = n;
	v->elements = scalloc(n, sizeof(float));

	return (v);
}

/* Destroys a vector. */
void vector_destroy(struct vector *v) {
	free(v->elements);
	free(v);
}

/* Fills up vector with random numbers. */
struct vector *vector_random(struct vector *v) {
	int i;
	
	/* Fill vector. */
	for (i = 0; i < vector_size(v); i++)
		VECTOR(v, i) = randnum() & 0xffff;

	return (v);
}