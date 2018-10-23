#ifndef _SLAVE_H_
#define _SLAVE_H_

#define NUM_THREADS (NUM_CORES/NUM_CLUSTERS)

#define DIMENSION          16  /* Universal dimension.      */
#define NUM_POINTS     262144  /* Max. number of points.    */
#define NUM_CENTROIDS    1024  /* Max. number of centroids. */

/* Size of arrays. */
#define MAP_SIZE (NUM_POINTS/NUM_CLUSTERS)                /* map[]         */
#define POINTS_SIZE ((NUM_POINTS/NUM_CLUSTERS)*DIMENSION) /* points[]      */
#define CENTROIDS_SIZE (NUM_CENTROIDS*DIMENSION)          /* centroids[]   */
#define PPOPULATION_SIZE (NUM_CENTROIDS)                  /* ppopulation[] */
#define LCENTROIDS_SIZE ((NUM_CENTROIDS/1)*DIMENSION)     /* lcentroids[]  */

/* Delta of array sizes. */
#define DELTA (NUM_CLUSTERS - 1)

/* Returns the ith data point. */
#define POINT(i) \
	(&points[(i)*dimension])

/* Returns the ith centroid. */
#define CENTROID(i) \
	(&centroids[(i)*dimension])

/* Returns the population for clusther ith, vector jth. */
#define POPULATION(i, j) \
	(&ppopulation[(i)*lncentroids[rank] + (j)])

/* Returns the jth input centroid from the ith cluster. */
#define PCENTROID(i, j) \
	(&centroids[((i)*lncentroids[rank] + (j))*dimension])

/* Returns the ith local centroid. */
#define LCENTROID(i) \
	(&lcentroids[(i)*dimension])

/*
 * Calculates the distance between two points.
*/
extern float vector_distance(float *a, float *b);
	
/*
 * Adds two vectors.
*/
extern float *vector_add(float *v1, const float *v2);
	
/*
* Multiplies a vector by a scalar.
*/
extern float *vector_mult(float *v, float scalar);
	
/*
 * Assigns a vector to another.
 */
extern float *vector_assign(float *v1, const float *v2);
	
/*
 * Tests if two vectors are equal.
 */
extern int vector_equal(const float *v1, const float *v2);

#endif /* SLAVE_H_ */