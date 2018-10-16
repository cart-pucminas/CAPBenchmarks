#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <arch.h>
#include <stdlib.h>

/* Vector. */
struct vector {
	int size;        /* Size.     */
	float *elements; /* Elements. */
};

/* Opaque pointer to a vector. */
typedef struct vector * vector_t;
	
/* Opaque pointer to a constant vector. */
typedef const struct vector * const_vector_t;
	
/* Returns the size of a vector. */
#define vector_size(v) \
	(((vector_t) (v))->size)

/* Returns the element [i] in a vector. */
#define VECTOR(v, i) \
	(((vector_t)(v))->elements[(i)])
	
/* Creates a vector. */
extern vector_t vector_create(int n);

/* Destroys a vector. */
extern void vector_destroy(vector_t v);

/* Fills up vector with random numbers. */
extern vector_t vector_random(vector_t v);

#endif /* _VECTOR_H_ */