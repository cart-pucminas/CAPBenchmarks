#ifndef _KM_H_
#define _KM_H_

#define CENTROIDS(i) \
	(&centroids[(i)*dimension])

#define POINTS(i) \
	(&points[(i)*dimension])

#define TMP_CENTROIDS(i) \
	(tmp[(i)*dimension])

/* Clusters points. */
int *kmeans(float *_points, int npoints, int ncentroids, int dimension);

/* Assigns a vector to another. */
extern void vector_assign(float *v1, float *v2);

/* Computes the euclidean distance between two points. */
extern float vector_distance(float *v1, float *v2);

/* Multiplies a vector by a scalar. */
extern void vector_mult(float *v, float scalar);

/* Adds two vectors. */
extern void vector_add(float *v1, float *v2);

#endif