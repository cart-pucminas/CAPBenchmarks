/* Kernel Includes */
#include <async_util.h>
#include <timer.h>
#include <arch.h>
#include <util.h>
#include "../km.h"

/* C And MPPA Library Includes*/
#include <omp.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

/* Data exchange segments. */
static mppa_async_segment_t infos_seg;
static mppa_async_segment_t var_off_seg;
static mppa_async_segment_t centroids_seg;

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
uint64_t total = 0;          /* Time spent on slave.    */
uint64_t communication = 0;  /* Time spent on comms.    */               
size_t data_put = 0;         /* Number of bytes put.    */
size_t data_get = 0;         /* Number of bytes gotten. */
unsigned nput = 0;           /* Number of put ops.      */
unsigned nget = 0;           /* Number of get ops.      */

/* Compute Cluster ID */
int cid;

#define VAR_OFF_SEG 3

struct offsets{
	int points, map, has_changed;
	int ppopulation, pcentroids;			
};

/* Variable offsets auxiliary. */
static struct offsets var_offsets;

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
 *                                 sync()                                    *
 *============================================================================*/

/* Sync with IO and asserts if another iteration is needed. */
static int sync() {
	int ret = 0; /* Is another iteration needed? */
	int has_changed_aux = 0;

	for (int i = 0; i < NUM_THREADS; i++) {
		if (has_changed[i]) {
			has_changed_aux = 1;
			break;
		}
	}

	dataPut(centroids, MPPA_ASYNC_DDR_0, var_offsets.pcentroids, ncentroids*dimension, sizeof(float), NULL);
	dataPut(ppopulation, MPPA_ASYNC_DDR_0, var_offsets.ppopulation, ncentroids, sizeof(int), NULL);
	dataPut(&has_changed_aux, MPPA_ASYNC_DDR_0, var_offsets.has_changed, 1, sizeof(int), NULL);

	send_signal();
	waitCondition(&io_signal, 0, MPPA_ASYNC_COND_GT, NULL);

	if (io_signal == 1) {
		dataGet(centroids, &centroids_seg, 0, ncentroids*dimension, sizeof(float), NULL);
		ret = 1;
	}

	io_signal = 0;

	return ret;
}

/*============================================================================*
 *                                kmeans()                                    *
 *============================================================================*/

/* Clusters data. */
static void kmeans() {	
	omp_set_num_threads(NUM_THREADS);
	for (int i = 0; i < NUM_THREADS; i++)
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

static void clone_segments() {
	cloneSegment(&signals_offset_seg, SIG_SEG_0, 0, 0, NULL);
	cloneSegment(&infos_seg, MSG_SEG_0, 0, 0, NULL);
	cloneSegment(&var_off_seg, VAR_OFF_SEG, 0, 0, NULL);
	cloneSegment(&centroids_seg, 6, 0, 0, NULL);
}

static void sync_offsets() {

	/* Offset in bytes. */
	off64_t points_aux, map_aux, has_changed_aux;
	off64_t ppopulation_aux, pcentroids_aux; 

	async_smalloc(MPPA_ASYNC_DDR_0, lnpoints*dimension*sizeof(float), &points_aux, NULL);
	async_smalloc(MPPA_ASYNC_DDR_0, lnpoints*sizeof(int), &map_aux, NULL);
	async_smalloc(MPPA_ASYNC_DDR_0, sizeof(int), &has_changed_aux, NULL);
	async_smalloc(MPPA_ASYNC_DDR_0, ncentroids*sizeof(int), &ppopulation_aux, NULL);
	async_smalloc(MPPA_ASYNC_DDR_0, ncentroids*dimension*sizeof(float), &pcentroids_aux, NULL);

	/* Converting offsets to positions. */
	var_offsets.points = points_aux/sizeof(float);
	var_offsets.map = map_aux/sizeof(int);
	var_offsets.has_changed = has_changed_aux/sizeof(int);
	var_offsets.ppopulation = ppopulation_aux/sizeof(int);
	var_offsets.pcentroids = pcentroids_aux/sizeof(float);

	dataPut(&var_offsets, &var_off_seg, cid, 1, sizeof(struct offsets), NULL);

	/* Send io_signal offset to IO. */
	send_sig_offset();
}

/* Initialize all remaining variables. */
static void init_variables() {
	points =  smalloc(lnpoints*dimension*sizeof(float));
	centroids = smalloc(ncentroids*dimension*sizeof(float));
	map = smalloc(lnpoints*sizeof(int));
	ppopulation = smalloc(ncentroids*sizeof(int));
}

/* Receives work from master process. */
static void get_work() {
	wait_signal();

	dataGet(points, MPPA_ASYNC_DDR_0, var_offsets.points, lnpoints*dimension, sizeof(float), NULL);
	dataGet(map, MPPA_ASYNC_DDR_0, var_offsets.map, lnpoints, sizeof(int), NULL);
	dataGet(centroids, &centroids_seg, 0, ncentroids*dimension, sizeof(float), NULL);
}

/*============================================================================*
 *                                  main()                                    *
 *============================================================================*/

static void send_result() {
	dataPut(map, MPPA_ASYNC_DDR_0, var_offsets.map, lnpoints, sizeof(int), NULL);

	/* Put statistics in stats. segment on IO side. */
	send_statistics(&infos_seg);

}

/* Clusters data. */
int main (__attribute__((unused))int argc, char **argv) {
	/* Initializes async client */
	async_slave_init();

	/* Timer synchronization */
	timer_init();

	/* Util information for the problem. */
	cid = __k1_get_cluster_id();
	nprocs = atoi(argv[0]);
	ncentroids = atoi(argv[1]);
	dimension = atoi(argv[2]);
	lnpoints = atoi(argv[3]);
	sigback_offset = (off64_t) atoll(argv[4]);

	/* Clones message exchange and io_signal segments */
	clone_segments();

	/* Synchronize variables offsets between Master and Slaves. */
	sync_offsets();

	/* Initialize all remaining variables. */
	init_variables();

	/* Get work from IO. */
	get_work();

	/* Start of km solving. */
	kmeans();

	/* Sends mapping result and statistics to IO. */
	send_result();

	/* Finalizes async library and rpc client */
	async_slave_finalize();

	return 0;
}