/* Kernel Include */
#include <util.h>
#include "km.h"

/* C And MPPA Library Includes*/
#include <math.h>
#include <string.h>

extern int dimension; /* Problem dimension. */

/* Assigns a vector to another. */
void vector_assign(float *v1, float *v2) {
	for (int i = 0; i < dimension; i++)
		v1[i] = v2[i];
}

/* Computes the euclidean distance between two points. */
float vector_distance(float *v1, float *v2) {
	float distance = 0;

	/* Computes the euclidean distance. */
	for (int i = 0; i < dimension; i++)
		distance +=  pow(v1[i] - v2[i], 2);
	distance = sqrt(distance);
	
	return distance;
}

/* Multiplies a vector by a scalar. */
void vector_mult(float *v, float scalar) {
	for (int i = 0; i < dimension; i++)
		v[i] *= scalar;
}

/* Adds two vectors. */
void vector_add(float *v1, float *v2) {
	for (int i = 0; i < dimension; i++)
		v1[i] += v2[i];
}
