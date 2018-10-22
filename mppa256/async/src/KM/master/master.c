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
static int npoints;       /* Number of data points.     */
static int dimension;     /* Dimension of data points.  */
static float mindistance; /* Minimum distance.          */
static int ncentroids;    /* Number of centroids.       */
static int *lnpoints;     /* Local number of points.    */
static vector_t *data;    /* Data points.               */
static float *centroids;  /* Data centroids.            */
static int *map;          /* Map of clusters.           */
static int *too_far;      /* Are points too far?        */
static int *has_changed;  /* Has any centroid changed?  */
static int *lncentroids;  /* Local number of centroids. */
static int *ppopulation;  /* Partial population.        */
static int *population;   /* Population of centroids.   */
static float *pcentroids; /* Partial centroids.         */

/* Timing auxiliars */
static uint64_t start, end;

#define VAR_OFF_SEG 3

struct offsets {
	off64_t points, centroids;
	off64_t map, too_far, has_changed;
	off64_t lncentroids, ppopulation;			
};

/* Variable offsets auxiliary. */
static struct offsets var_offsets[NUM_CLUSTERS+1];

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
	lnpoints = smalloc(nclusters*sizeof(int));
	centroids = smalloc(CENTROIDS_SIZE*sizeof(float));
	map = scalloc(npoints, sizeof(int));
	too_far = smalloc(nclusters*NUM_THREADS*sizeof(int));
	has_changed = smalloc(nclusters*NUM_THREADS*sizeof(int));
	lncentroids = smalloc(nclusters*sizeof(int));
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

/* Variables offset exchange between IO and Clusters. */
static void sync_offsets() {
	/* Get all clusters signal offset. */
	get_slaves_signals_offset();

	for (int i = 0; i < nclusters; i++)
		wait_signal(i);
}

/* Send work to clusters. */
static void send_work() {
	//#pragma omp parallel for default(shared) num_threads(3)
	for (int i = 0; i < nclusters; i++) {
		dataPut(lncentroids, MPPA_ASYNC_DDR_0, var_offsets[i].lncentroids, nclusters, sizeof(int), NULL);
		

		send_signal(i);
	}
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
	sync_offsets();

	/* Send work to slaves. */
	send_work();

	/* Wait all slaves statistics info. */
	wait_statistics();

	/* Set slaves statistics. */
	set_statistics(statistics);

	/* Waiting for PE0 of each cluster to end */
	join_slaves();

	/* Finalizes async server */
	async_master_finalize();

	/* House keeping. */
	free(lncentroids);
	free(lnpoints);
	free(ppopulation);
	free(pcentroids);
	free(population);
	free(centroids);
	free(has_changed);
	free(too_far);
	
	return (map);
}