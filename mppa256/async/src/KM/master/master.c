/* Kernel Includes */
#include <async_util.h>
#include <message.h>
#include <global.h>
#include <timer.h>
#include <util.h>
#include "../km.h"

/* C And MPPA Library Includes*/
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/* Asynchronous segments */
static mppa_async_segment_t infos_seg;
static mppa_async_segment_t var_off_seg;
static mppa_async_segment_t centroids_seg;

/* Statistics information from slaves. */
static struct message statistics[NUM_CLUSTERS];

/* K-means. */
int dimension;              			/* Dimension of data points.  */
static int npoints;              	  	/* Number of data points.     */
static int ncentroids;            		/* Number of centroids.       */
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

#define VAR_OFF_SEG 3

struct offsets{
	int points, map, has_changed;
	int ppopulation, pcentroids;			
};

/* Variable offsets auxiliary. */
static struct offsets var_offsets[NUM_CLUSTERS];

/* Create segments for data and info exchange. */
static void create_segments() {
	createSegment(&signals_offset_seg, SIG_SEG_0, &sig_offsets, nclusters * sizeof(off64_t), 0, 0, NULL);
	createSegment(&infos_seg, MSG_SEG_0, &statistics, nclusters * sizeof(struct message), 0, 0, NULL);
	createSegment(&var_off_seg, VAR_OFF_SEG, &var_offsets, nclusters * sizeof(struct offsets), 0, 0, NULL);
	createSegment(&centroids_seg, 6, centroids, ncentroids*dimension * sizeof(float), 0, 0, NULL);
}

static void spawnSlaves() {
	start = timer_get();

	char str_nclusters[10], str_ncentroids[10], str_dim[10];
	sprintf(str_nclusters, "%d", nclusters);
	sprintf(str_ncentroids, "%d", ncentroids);
	sprintf(str_dim, "%d", dimension);

	set_cc_signals_offset();

	/* Parallel spawning PE0 of cluster "i" */
	#pragma omp parallel for default(shared) num_threads(3)
	for (int i = 0; i < nclusters; i++) {
		char str_lnpoints[10];
		sprintf(str_lnpoints, "%d", lnpoints[i]);

		char *args[6];
		args[0] = str_nclusters;
		args[1] = str_ncentroids;
		args[2] = str_dim;
		args[3] = str_lnpoints;
		args[4] = str_cc_signals_offset[i];
		args[5] = NULL;

		spawn_slave(i, args);
	}
	end = timer_get();
	spawn = timer_diff(start, end);
}

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
	int count = 0; /* Position counter. */
	for (int i = 0; i < nclusters; i++) {
		dataPut(&points[count*dimension], MPPA_ASYNC_DDR_0, var_offsets[i].points, lnpoints[i]*dimension, sizeof(float), NULL);
		dataPut(&map[count], MPPA_ASYNC_DDR_0, var_offsets[i].map, lnpoints[i], sizeof(int), NULL);
		count += lnpoints[i];
		send_signal(i);
	}
}

/* Sync with CC and asserts if another iteration is needed. */
static int sync() {
	int i, j;   /* Loop indexes. */
	int again;  /* Loop again? */
	int sigaux; /* auxiliar signal for clusters. */
	
	for (i = 0; i < nclusters; i++) {
		wait_signal(i);
		dataGet(PCENTROID(i,0), MPPA_ASYNC_DDR_0, var_offsets[i].pcentroids, ncentroids*dimension, sizeof(float), NULL);
		dataGet(PPOPULATION(i,0), MPPA_ASYNC_DDR_0, var_offsets[i].ppopulation, ncentroids, sizeof(int), NULL);
		dataGet(&has_changed[i], MPPA_ASYNC_DDR_0, var_offsets[i].has_changed, 1, sizeof(int), NULL);
	}

	start = timer_get();

	/* Clear all centroids and population for recalculation. */
	memset(centroids, 0, ncentroids*dimension*sizeof(float));
	memset(population, 0, ncentroids*sizeof(int));

	#pragma omp parallel for private(i, j) default(shared) num_threads(3)
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
	sigaux = (again) ? 1 : 2;

	end = timer_get();
	master += timer_diff(start, end);

	for (i = 0; i < nclusters; i++)
		poke(mppa_async_default_segment(i), sig_offsets[i], sigaux);

	return again;
}

/* Gets mapping result and statistics to IO. */
static void get_results() {
	/* Wait all slaves statistics info. */
	wait_statistics();

	/* Get map result. */
	int counter = 0; /* Points counter. */
	for (int i = 0; i < nclusters; i++) {
		dataGet(&map[counter], MPPA_ASYNC_DDR_0, var_offsets[i].map, lnpoints[i], sizeof(int), NULL);
		counter += lnpoints[i];
	}
}

/* Clusters data. */
int *kmeans(float *_points, int _npoints, int _ncentroids, int _dimension) {
	/* Initializes async server */
	async_master_start();

	/* Setup parameters. */
	points = _points;
	npoints = _npoints;
	ncentroids = _ncentroids;
	dimension = _dimension;
	initialize_variables();

	/* Distribute work among clusters. */
	for (int i = 0; i < nclusters; i++)
		lnpoints[i] = ((i + 1) < nclusters) ?  npoints/nclusters :  npoints - i*(npoints/nclusters);

	/* Creates all necessary segments for data exchange */
	create_segments();

	/* Spawns all clusters getting signal offsets after. */
	spawnSlaves();

	/* Synchronize variable offsets between Master and Slaves. */
	get_slaves_signals_offset();

	/* Send work to slaves. */
	send_work();

	/* Iterations until finished. */
	do {} while (sync());

	/* Gets mapping result and statistics to IO. */
	get_results();

	/* Set slaves statistics. */
	set_statistics(statistics);

	/* Waiting for PE0 of each cluster to end */
	join_slaves();

	/* Finalizes async server */
	async_master_finalize();

	/* House keeping. */
	free(ppopulation);
	free(pcentroids);
	free(population);
	free(centroids);

	return (map);
}