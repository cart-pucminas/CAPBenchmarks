/*
* Copyright(C) 2011-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
* 
* master.c - Master process.
*/

#include <assert.h>
#include <global.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>
#include <stdio.h>
#include "../km.h"

/*
 * Wrapper to data_send(). 
 */
#define data_send(a, b, c)                   \
	{                                        \
		data_sent += c;                      \
		nsend++;                             \
		communication += data_send(a, b, c); \
	}                                        \

/*
 * Wrapper to data_receive(). 
 */
#define data_receive(a, b, c)                   \
	{                                           \
		data_received += c;                     \
		nreceive++;                             \
		communication += data_receive(a, b, c); \
	}                                           \

/* K-means. */
int dimension;              			/* Dimension of data points.  */
static int npoints;                		/* Number of data points.     */
static int ncentroids;             		/* Number of centroids.       */
static float *points;              		/* Data points.               */
static float *centroids;           		/* Data centroids.            */
static int *map;                   		/* Map of clusters.           */
static int *population;            		/* Population of centroids.   */
static int *ppopulation;           		/* Partial population.        */
static float *pcentroids;          		/* Partial centroids.         */
static int lnpoints[NUM_CLUSTERS]; 		/* Local number of points.    */
static int has_changed[NUM_CLUSTERS]; 	/* Has any centroid changed?  */

/* Timing auxiliars */
static uint64_t start, end;


/* Memory allocation and proper initialization. */
static void initialize_variables() {
	int i, j; /* Loop indexes. */
	
	/* Create auxiliary variables. */
	map = scalloc(npoints, sizeof(int));
	centroids = smalloc(ncentroids*dimension*sizeof(float));
	pcentroids = smalloc(ncentroids*dimension*nclusters*sizeof(float));
	population = smalloc(ncentroids*sizeof(int));
	ppopulation = smalloc(ncentroids*nclusters*sizeof(int));
	
	start = timer_get();

	/* Initialize mapping. */
	for (i = 0; i < npoints; i++)
		map[i] = -1;

	/* Initialize centroids. */
	for (i = 0; i < ncentroids; i++) {
		j = randnum()%npoints;
		memcpy(CENTROID(i), POINT(j), dimension*sizeof(float));
		map[j] = i;
	}

	/* Map unmapped data points. */
	for (i = 0; i < npoints; i++) {
		if (map[i] < 0)
			map[i] = randnum()%ncentroids;
	}

	end = timer_get();
	master += timer_diff(start, end);
}

/* Send work to clusters. */
static void send_work() {
	int i; 			/* Loop index. */
	int count = 0; 	/* Position counter. */

	for (i = 0; i < nclusters; i++) {
		/* Util information for the problem. */
		data_send(outfd[i], &nclusters, sizeof(int));
		data_send(outfd[i], &ncentroids, sizeof(int));
		data_send(outfd[i], &dimension, sizeof(int));
		data_send(outfd[i], &lnpoints[i], sizeof(int));

		/* Actual initial tasks. */
		data_send(outfd[i], &points[count * dimension], lnpoints[i] * dimension * sizeof(float));
		data_send(outfd[i], &map[count], lnpoints[i] * sizeof(int));
		data_send(outfd[i], centroids, ncentroids * dimension * sizeof(float));

		count += lnpoints[i];
	}
}

/* Sync with CC and asserts if another iteration is needed. */
static int sync() {
	int i, j;   /* Loop indexes. */
	int again;  /* Loop again? */
	int sigaux; /* auxiliar signal for clusters. */

	for (i = 0; i < nclusters; i++) {
		data_receive(infd[i], PCENTROID(i,0), ncentroids * dimension * sizeof(float));
		data_receive(infd[i], PPOPULATION(i,0), ncentroids * sizeof(int));
		data_receive(infd[i], &has_changed[i], sizeof(int));
	}

	start = timer_get();

	/* Clear all centroids and population for recalculation. */
	memset(centroids, 0, ncentroids*dimension*sizeof(float));
	memset(population, 0, ncentroids*sizeof(int));

	for (i = 0; i < ncentroids; i++) {
		for (j = 0; j < nclusters; j++) {
			vector_add(CENTROID(i), PCENTROID(j, i));
			population[i] += *PPOPULATION(j, i);
		}
		vector_mult(CENTROID(i), 1.0/population[i]);
	}

	for (i = 0; i < nclusters; i++)
		if (has_changed[i]) break;

	again = (i < nclusters) ? 1 : 0;

	end = timer_get();
	master += timer_diff(start, end);

	for (i = 0; i < nclusters; i++) {
		data_send(outfd[i], &again, sizeof(int));
		if (again == 1) {
			data_send(outfd[i], centroids, ncentroids * dimension * sizeof(float));
		}
	}

	return again;
}

/* Gets mapping result and statistics to IO. */
static void get_results() {
	int counter = 0; /* Points counter. */
	int i;           /* Loop index.     */

	for (i = 0; i < nclusters; i++) {
		data_receive(infd[i], &map[counter], lnpoints[i] * sizeof(int));
		counter += lnpoints[i];
	}
}

/* Clusters data. */
int *kmeans(float *_points, int _npoints, int _ncentroids, int _dimension) {
	int i; /* Loop index. */

	/* Setup parameters. */
	points = _points;
	npoints = _npoints;
	ncentroids = _ncentroids;
	dimension = _dimension;
	initialize_variables();

	/* Distribute work among clusters. */
	for (i = 0; i < nclusters; i++)
		lnpoints[i] = ((i + 1) < nclusters) ?  npoints/nclusters :  npoints - i*(npoints/nclusters);
	
	/* Initializes ipc and spawn all clusters. */
	open_noc_connectors();

	start = timer_get();
	spawn_slaves();
	end = timer_get();
	spawn = timer_diff(start, end);

	/* Send work to slaves. */
	send_work();

	/* Iterations until finished. */
	do {} while (sync());

	/* Gets mapping result and statistics to IO. */
	get_results();

	/* Waiting for PE0 of each cluster to end. */
	join_slaves();

	/* Finalizes ipc. */
	close_noc_connectors();

	/* House keeping. */
	free(ppopulation);
	free(pcentroids);
	free(population);
	free(centroids);
	
	return (map);
}
