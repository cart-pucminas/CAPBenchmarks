/*
 * Copyright(C) 2011-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <globl.h> - Global constant and variables.
 */

#ifndef GLOBL_H_
#define GLOBL_H_

	#include <mppaipc.h>

	/* Parameters. */
	#define NUM_PROCS         16 /* Number of processes.     */
	#define NUM_THREADS       16 /* Number of threads.       */
	#define NUM_POINTS    262144 /* Number of data points.   */
	#define NUM_CENTROIDS   1024 /* Number of centroids.     */
	#define DIMENSION         16 /* Dimension of datapoints. */
	#define SEED               0 /* Seed value.              */
	#define MINDISTANCE      0.0 /* Minimum distance.        */

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
