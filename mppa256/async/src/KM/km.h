#ifndef _KM_H_
#define _KM_H_

/* Returns the ith centroid. */
#define CENTROID(i) \
	(&centroids[(i)*dimension])

/* Returns the ith points. */
#define POINT(i) \
	(&points[(i)*dimension])
	
#define PCENTROID(i, j) \
(&pcentroids[(i)*ncentroids*dimension + (j)*dimension])

#define PPOPULATION(i, j) \
(&ppopulation[(i)*ncentroids + (j)])

/* Assigns a vector to another. */
extern void vector_assign(float *v1, float *v2);

/* Computes the euclidean distance between two points. */
extern float vector_distance(float *v1, float *v2);

/* Multiplies a vector by a scalar. */
extern void vector_mult(float *v, float scalar);

/* Adds two vectors. */
extern void vector_add(float *v1, float *v2);

#endif