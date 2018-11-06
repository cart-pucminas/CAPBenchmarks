/* Kernel Includes */
#include <async_util.h>
#include <message.h>
#include <global.h>
#include <timer.h>
#include <util.h>
#include "master.h"
#include "vector.h"

/* C And MPPA Library Includes*/
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/* Statistics and variable offsets segment. */
static mppa_async_segment_t infos_seg;
static mppa_async_segment_t var_off_seg;

/* Statistics information from slaves. */
static struct message statistics[NUM_CLUSTERS];

/* K-means. */
static int npoints;                    /* Number of data points.     */
static int dimension;                  /* Dimension of data points.  */
static float mindistance;              /* Minimum distance.          */
static int ncentroids;                 /* Number of centroids.       */
static vector_t *data;                 /* Data points.               */
static float *centroids;               /* Data centroids.            */
static int *map;                       /* Map of clusters.           */
static int *population;                /* Population of centroids.   */
static int *ppopulation;               /* Partial population.        */
static float *pcentroids;              /* Partial centroids.         */
static int too_far[NUM_CORES];         /* Are points too far?        */
static int lnpoints[NUM_CLUSTERS];     /* Local number of points.    */
static int has_changed[NUM_CORES];     /* Has any centroid changed?  */
static int lncentroids[NUM_CLUSTERS];  /* Local number of centroids. */

/* Timing auxiliars */
static uint64_t start, end;

#define VAR_OFF_SEG 3

struct offsets{
	int points, centroids;
	int map, too_far, has_changed;
	int lncentroids, ppopulation, lcentroids;			
};

/* Variable offsets auxiliary. */
static struct offsets var_offsets[NUM_CLUSTERS];

/* Create segments for data and info exchange. */
static void create_segments() {
	createSegment(&signals_offset_seg, SIG_SEG_0, &sig_offsets, nclusters * sizeof(off64_t), 0, 0, NULL);
	createSegment(&infos_seg, MSG_SEG_0, &statistics, nclusters * sizeof(struct message), 0, 0, NULL);
	createSegment(&var_off_seg, VAR_OFF_SEG, &var_offsets, (nclusters+1) * sizeof(struct offsets), 0, 0, NULL);
}

static void spawnSlaves() {
	start = timer_get();

	char str_nclusters[10], str_ncentroids[10], str_mindist[10], str_dim[10];
	sprintf(str_nclusters, "%d", nclusters);
	sprintf(str_ncentroids, "%d", ncentroids);
	sprintf(str_mindist, "%f", mindistance);
	sprintf(str_dim, "%d", dimension);

	set_cc_signals_offset();

	/* Parallel spawning PE0 of cluster "i" */
	#pragma omp parallel for default(shared) num_threads(3)
	for (int i = 0; i < nclusters; i++) {
		char str_lnpoints[10];
		sprintf(str_lnpoints, "%d", lnpoints[i]);

		char *args[7];
		args[0] = str_nclusters;
		args[1] = str_ncentroids;
		args[2] = str_mindist;
		args[3] = str_dim;
		args[4] = str_lnpoints;
		args[5] = str_cc_signals_offset[i];
		args[6] = NULL;

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
	centroids = smalloc(CENTROIDS_SIZE*sizeof(float));
	ppopulation = smalloc(PPOPULATION_SIZE*sizeof(int));
	population = smalloc(POPULATION_SIZE*sizeof(int));
	pcentroids = smalloc(PCENTROIDS_SIZE*sizeof(float));
	
	start = timer_get();
	/* Initialize mapping. */
	for (i = 0; i < npoints; i++)
		map[i] = -1;

	/* Initialize centroids. */
	for (i = 0; i < ncentroids; i++) {
		j = randnum()%npoints;
		memcpy(CENTROID(i), data[j]->elements, dimension*sizeof(float));
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

/* Distribute work among slave processes. */
static void distribute_work() {
	for (int i = 0; i < nclusters; i++) {
		lnpoints[i] = ((i + 1) < nclusters) ? 
			npoints/nclusters :  npoints - i*(npoints/nclusters);
			
		lncentroids[i] = ((i + 1) < nclusters) ? 
			ncentroids/nclusters : ncentroids - i*(ncentroids/nclusters);
	}
}

/* Send work to clusters. */
static void send_work() {
	for (int i = 0; i < nclusters; i++) {
		dataPut(lncentroids, MPPA_ASYNC_DDR_0, var_offsets[i].lncentroids, nclusters, sizeof(int), NULL);

		for (int j = 0; j < lnpoints[i]; j++) 
			dataPut(data[i*(npoints/nclusters)+j]->elements, MPPA_ASYNC_DDR_0, var_offsets[i].points, dimension, sizeof(float), NULL);

		dataPut(centroids, MPPA_ASYNC_DDR_0, var_offsets[i].centroids, ncentroids*dimension, sizeof(float), NULL);

		dataPut(&map[i*(npoints/nclusters)], MPPA_ASYNC_DDR_0, var_offsets[i].map, lnpoints[i], sizeof(int), NULL);

		send_signal(i);
	}
}

/* Synchronizes partial centroids. */
static void sync_pcentroids() {
	int i, j;            /* Loop indexes.          */
	
	/* Receive partial centroids. */
	for (i = 0; i < nclusters; i++) {
		wait_signal(i);
		dataGet(PCENTROID(i,0), MPPA_ASYNC_DDR_0, var_offsets[i].centroids, ncentroids*dimension, sizeof(float), NULL);
	}

	/* 
	 * Send partial centroids to the
	 * slave process that is assigned to it.
	 */
	for (i = 0; i < nclusters; i++) {
		/* Build partial centroid. */
		start = timer_get();
		for (j = 0; j < nclusters; j++)
			memcpy(CENTROID(j*lncentroids[i]), PCENTROID(j, i*(ncentroids/nclusters)), lncentroids[i]*dimension*sizeof(float));
		end = timer_get();
		master += timer_diff(start, end);

		dataPut(centroids, MPPA_ASYNC_DDR_0, var_offsets[i].centroids, nclusters*lncentroids[i]*dimension, sizeof(float), NULL);
		send_signal(i);
	}
}

/* Synchronizes partial population. */
static void sync_ppopulation() {
	int i, j;            /* Loop indexes.          */

	/* Receive temporary population. */
	for (i = 0; i < nclusters; i++) {
		wait_signal(i);
		dataGet(PPOPULATION(i,0), MPPA_ASYNC_DDR_0, var_offsets[i].ppopulation, ncentroids, sizeof(int), NULL);
	}

	/* 
	 * Send partial population to the
	 * slave process that assigned to it.
	 */
	for (i = 0; i < nclusters; i++) {
		/* Build partial population. */
		start = timer_get();
		for (j = 0; j < nclusters; j++)
			memcpy(&population[j*lncentroids[i]], PPOPULATION(j, i*(ncentroids/nclusters)), lncentroids[i]*sizeof(int));
		end = timer_get();
		master += timer_diff(start, end);

		dataPut(ppopulation, MPPA_ASYNC_DDR_0, var_offsets[i].ppopulation, nclusters*lncentroids[i], sizeof(int), NULL);
		send_signal(i);
	}
}

/* Synchronizes centroids. */
static void sync_centroids(void) {
	int i;     /* Loop index.            */

	/* Receive centroids. */
	for (i = 0; i < nclusters; i++) {
		wait_signal(i);
		dataGet(CENTROID(i*(ncentroids/nclusters)), MPPA_ASYNC_DDR_0, var_offsets[i].lcentroids, lncentroids[i]*dimension, sizeof(float), NULL);
	}

	/* Broadcast centroids. */
	for (i = 0; i < nclusters; i++) {
		dataPut(centroids, MPPA_ASYNC_DDR_0, var_offsets[i].centroids, ncentroids*dimension, sizeof(float), NULL);
		send_signal(i);
	}
}

/* Synchronizes slaves' status. */
static void sync_status() {
	int i;     /* Loop index.            */

	/* Receive data. */
	for (i = 0; i < nclusters; i++) {
		wait_signal(i);
		dataGet(&has_changed[i*NUM_THREADS], MPPA_ASYNC_DDR_0, var_offsets[i].has_changed, NUM_THREADS, sizeof(int), NULL);
		dataGet(&too_far[i*NUM_THREADS], MPPA_ASYNC_DDR_0, var_offsets[i].too_far, NUM_THREADS, sizeof(int), NULL);
	}

	/* Broadcast data to slaves. */
	for (i = 0; i < nclusters; i++) {
		dataPut(has_changed, MPPA_ASYNC_DDR_0, var_offsets[i].has_changed, nclusters*NUM_THREADS, sizeof(int), NULL);
		dataPut(too_far, MPPA_ASYNC_DDR_0, var_offsets[i].too_far, nclusters*NUM_THREADS, sizeof(int), NULL);
		send_signal(i);
	}
}

/* Asserts if another iteration is needed. */
static int again() {
	start = timer_get();
	for (int i = 0; i < nclusters*NUM_THREADS; i++) {
		if (has_changed[i] && too_far[i]) {
			end = timer_get();
			master += timer_diff(start, end);
			return (1);
		}
	}
	end = timer_get();
	master += timer_diff(start, end);
	
	return (0);
}

/* Gets mapping result and statistics to IO. */
static void get_results() {
	for (int i = 0; i < nclusters; i++) {
		/* Wait data is ready signal from cc. */
		wait_signal(i);

		dataGet(&map[i*(npoints/nclusters)], MPPA_ASYNC_DDR_0, var_offsets[i].map, lnpoints[i], sizeof(int), NULL);

		/* Handshake before statistics exchange. */
		send_signal(i);
	}

	/* Wait all slaves statistics info. */
	wait_statistics();
}

/* Clusters data. */
int *kmeans(vector_t *_data, int _npoints, int _ncentroids, float _mindistance) {
	/* Initializes async server */
	async_master_start();

	/* Setup parameters. */
	data = _data;
	npoints = _npoints;
	ncentroids = _ncentroids;
	mindistance = _mindistance;
	dimension = vector_size(data[0]);
	initialize_variables();

	/* Distribute work among clusters. */
	distribute_work();

	/* Creates all necessary segments for data exchange */
	create_segments();

	/* Spawns all clusters getting signal offsets after. */
	spawnSlaves();

	/* Synchronize variable offsets between Master and Slaves. */
	get_slaves_signals_offset();

	/* Send work to slaves. */
	send_work();

	/* Data exchange. */
	do {
		sync_pcentroids();
		sync_ppopulation();
		sync_centroids();
		sync_status();
	} while (again());

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