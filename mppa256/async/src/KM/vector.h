#ifndef _VECTOR_H_
#define _VECTOR_H_

extern int dimension;
	
/* Fills up vector with random numbers. */
extern void vector_random(float *vector); 

/* Adds two vectors. */
extern void vector_add(float *v1, const float *v2);

/* Multiplies a vector by a scalar. */
extern void vector_mult(float *v, float scalar);

/* Assigns a vector to another. */
extern void vector_assign(float *v1, const float *v2);

/* Tests if two vectors are equal. */
extern int vector_equal(const float *v1, const float *v2);

/* Calculates the distance between two points. */
extern float vector_distance(float *a, float *b);

#endif /* _VECTOR_H_ */