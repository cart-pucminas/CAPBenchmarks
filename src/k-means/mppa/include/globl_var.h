/*
 * Copyright(C) 2011-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <globl.h> - Global constant and variables.
 */

#ifndef GLOBL_H_
#define GLOBL_H_

	#include <mppaipc.h>

	/* Parameters. */
	#define NUM_PROCS     NUM_PROCS_VAR     /* Number of processes (clusters). */
	#define NUM_THREADS   NUM_THREADS_VAR   /* Number of threads. per cluster  */
	#define NUM_POINTS    NUM_POINTS_VAR    /* Number of data points.          */
	#define NUM_CENTROIDS NUM_CENTROIDS_VAR /* Number of centroids.            */
	#define DIMENSION     16                /* Dimension of datapoints.        */
	#define SEED          SEED_VAR          /* Seed value.                     */
	#define MINDISTANCE   0.0               /* Minimum distance.               */

	/*
	 * Returns the ith data point.
	 */
	#define POINT(i) \
		(&points[(i)*DIMENSION])

	/*
	 * Returns the ith centroid.
	 */
	#define CENTROID(i) \
		(&centroids[(i)*DIMENSION])

#endif /* GLOBL_H_ */
