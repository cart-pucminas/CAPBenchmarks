#include <math.h>
#include <string.h>
#include <util.h>
#include "vector.h"

/* Fills up vector with random numbers. */
void vector_random(float *vector) {
	for (int i = 0; i < dimension; i++)
		vector[i] = randnum() & 0xffff;
}

/* Adds two vectors. */
void vector_add(float *v1, const float *v2) {
	for (int i = 0; i < dimension; i++)
		v1[i] += v2[i];
}

/* Multiplies a vector by a scalar. */
void vector_mult(float *v, float scalar) {
	for (int i = 0; i < dimension; i++)
		v[i] *= scalar;
}

/* Assigns a vector to another. */
void vector_assign(float *v1, const float *v2) {
	for (int i = 0; i < dimension; i++)
		v1[i] = v2[i];
}

/* Tests if two vectors are equal. */
int vector_equal(const float *v1, const float *v2) {
	for (int i = 0; i < dimension; i++) {
		if (v1[i] != v2[i])
			return (0);
	}
	return (1);
}

/* Calculates the distance between two points. */
float vector_distance(float *a, float *b) {
	float distance; /* Distance.   */
	distance = 0;

	/* Computes the euclidean distance. */
	for (int i = 0; i < dimension; i++)
		distance += pow(a[i] - b[i], 2);
	distance = sqrt(distance);
	
	return (distance);
}