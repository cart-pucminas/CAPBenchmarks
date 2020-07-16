/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * slave.c - k-means slave process.
 */

#include <arch.h>
#include <assert.h>
#include <omp.h>
#include <stdint.h>
#include <string.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>
#include <stdio.h>
#include "../km.h"

/* K-means. */
int dimension;                        /* Dimension of data points.    */
static int nprocs;					  /* Clusters spawned.            */
static int ncentroids;                /* Number of centroids.         */
static int lnpoints;                  /* Local number of points.      */
static float *points;                 /* Data points.                 */
static float *centroids;              /* Data centroids.              */
static int *map;                      /* Map of clusters.             */
static int *ppopulation;              /* Partial population.          */ 
static int has_changed[NUM_THREADS];  /* Has any centroid changed?    */

/* Thread communication. */
static omp_lock_t lock[NUM_THREADS];

/* Timing statistics auxiliars. */
static uint64_t start, end;

/* Individual Slave statistics. */
uint64_t communication = 0;
uint64_t total = 0;

/*============================================================================*
 *                                populate()                                 *
 *============================================================================*/

static void populate() {
	int tid;        /* Thread ID.             */
	int i, j;       /* Loop indexes.          */
	int lock_aux;   /* Lock auxiliar.         */
	int init_map;   /* Point initial mapping. */
	float tmp_dist; /* Temporary distance.    */
	float distance; /* Distance.              */

	start = timer_get();

	/* Reset variables for new calculation. */
	memset(ppopulation, 0, ncentroids*sizeof(int));
	memset(has_changed, 0, NUM_THREADS*sizeof(int));

	/* Iterate over data points. */
	#pragma omp parallel private(i, j, tmp_dist, distance, tid, init_map, lock_aux) default(shared)
	{
		tid = omp_get_thread_num();

		#pragma omp for
		for (i = 0; i < lnpoints; i++) {	
			distance = vector_distance(CENTROID(map[i]), POINT(i));
			init_map = map[i];

			/* Looking for closest cluster. */
			for (j = 0; j < ncentroids; j++) {	
				/* Point is in this cluster. */
				if (j == map[i])
					continue;
						
				tmp_dist = vector_distance(CENTROID(j), POINT(i));
					
				/* Found. */
				if (tmp_dist < distance) {
					map[i] = j;
					distance = tmp_dist;
				}
			}

			lock_aux = map[i] % NUM_THREADS;

			omp_set_lock(&lock[lock_aux]);
			ppopulation[map[i]]++;
			omp_unset_lock(&lock[lock_aux]);

			if (map[i] != init_map)
				has_changed[tid] = 1;
		}
	}

	end = timer_get();
	total += timer_diff(start, end);
}

/*============================================================================*
 *                          compute_centroids()                               *
 *============================================================================*/

/* Computes clusters partial centroids. */
static void compute_centroids() {
	int i;        /* Loop index.                 */
	int lock_aux; /* Lock auxiliar.              */

	start = timer_get();

	/* Clear all centroids for recalculation. */
	memset(CENTROID(0), 0, ncentroids*dimension*sizeof(float));

	/* Compute means. */
	#pragma omp parallel for private(i, lock_aux) default(shared)
	for (i = 0; i < lnpoints; i++) {
			lock_aux = map[i] % NUM_THREADS;
			omp_set_lock(&lock[lock_aux]);
			vector_add(CENTROID(map[i]), POINT(i));
			omp_unset_lock(&lock[lock_aux]);	
	}

	end = timer_get();
	total += timer_diff(start, end);
}

/*============================================================================*
 *                                 sync()                                     *
 *============================================================================*/

/* Sync with IO and asserts if another iteration is needed. */
static int sync() {
	int i;       /* Loop index. */
	int ret = 0; /* Is another iteration needed? */
	int has_changed_aux = 0;

	for (i = 0; i < NUM_THREADS; i++) {
		if (has_changed[i]) {
			has_changed_aux = 1;
			break;
		}
	}

	data_send(outfd, centroids, ncentroids * dimension * sizeof(float));
	data_send(outfd, ppopulation, ncentroids * sizeof(int));
	data_send(outfd, &has_changed_aux, sizeof(int));

	data_receive(infd, &ret, sizeof(int));

	if (ret == 1)
		data_receive(infd, centroids, ncentroids * dimension * sizeof(float));

	return ret;
}

/*============================================================================*
 *                                kmeans()                                    *
 *============================================================================*/

/* Clusters data. */
static void kmeans() {	
	int i; /* Loop index. */
	omp_set_num_threads(NUM_THREADS);
	for (i = 0; i < NUM_THREADS; i++)
		omp_init_lock(&lock[i]);

	/* Data exchange. */
	do {	
		populate();
		compute_centroids();
	} while (sync());
}

/*============================================================================*
 *                                 getwork()                                  *
 *============================================================================*/

/* Initialize all remaining variables. */
static void init_variables() {
	points =  smalloc(lnpoints * dimension * sizeof(float));
	centroids = smalloc(ncentroids * dimension * sizeof(float));
	map = smalloc(lnpoints * sizeof(int));
	ppopulation = smalloc(ncentroids * sizeof(int));
}

/* Receives work from master process. */
static void get_work() {
	data_receive(infd, points, lnpoints * dimension * sizeof(float));
	data_receive(infd, map, lnpoints * sizeof(int));
	data_receive(infd, centroids, ncentroids * dimension * sizeof(float));
}

/*============================================================================*
 *                                  main()                                    *
 *============================================================================*/

/* Clusters data. */
int main (__attribute__((unused))int argc, char **argv) {
	/* Timer synchronization */
	timer_init();

	/* Setup interprocess communication. */
	rank = atoi(argv[0]);
	open_noc_connectors();

	/* Util information for the problem. */
	data_receive(infd, &nprocs, sizeof(int));
	data_receive(infd, &ncentroids, sizeof(int));
	data_receive(infd, &dimension, sizeof(int));
	data_receive(infd, &lnpoints, sizeof(int));

	/* Initialize all remaining variables. */
	init_variables();
	
	/* Actual initial tasks. */
	get_work();
	
	/* Start of km solving. */
	kmeans();

	/* Sends mapping result and statistics to IO. */
	data_send(outfd, map, lnpoints * sizeof(int));
	data_send(outfd, &total, sizeof(uint64_t));
	
	close_noc_connectors();
	mppa_exit(0);

	return (0);
}
