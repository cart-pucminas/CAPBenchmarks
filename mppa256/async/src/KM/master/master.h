#ifndef _MASTER_H_
#define _MASTER_H_	

#define NUM_THREADS (NUM_CORES/NUM_CLUSTERS)

/* Size of arrays. */
#define CENTROIDS_SIZE ((ncentroids*dimension)+nclusters*(nclusters-1)*dimension)
#define POPULATION_SIZE (ncentroids + nclusters*(nclusters-1))
#define PCENTROIDS_SIZE (nclusters*ncentroids*dimension)
#define PPOPULATION_SIZE (nclusters*ncentroids)

/* Returns the ith centroid. */
#define CENTROID(i) \
	(&centroids[(i)*dimension])

#define PCENTROID(i, j) \
(&pcentroids[((i)*ncentroids + (j))*dimension])

#define PPOPULATION(i, j) \
(&ppopulation[(i)*ncentroids + (j)])


#endif /* _MASTER_H_ */