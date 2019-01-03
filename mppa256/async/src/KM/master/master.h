#ifndef _MASTER_H_
#define _MASTER_H_	

/* Returns the ith centroid. */
#define CENTROID(i) \
	(&centroids[(i)*dimension])

/* Returns the ith points. */
#define POINT(i) \
	(&point[(i)*dimension])
	
#define PCENTROID(i, j) \
(&pcentroids[((i)*ncentroids + (j))*dimension])

#define PPOPULATION(i, j) \
(&ppopulation[(i)*ncentroids + (j)])

#endif /* _MASTER_H_ */