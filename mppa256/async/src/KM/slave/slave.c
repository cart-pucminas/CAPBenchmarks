/* Kernel Includes */
#include <async_util.h>
#include <arch.h>
#include <timer.h>
#include <util.h>
#include "slave.h"

/* C And MPPA Library Includes*/
#include <omp.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

/* Data exchange segments. */
static mppa_async_segment_t infos_seg;
static mppa_async_segment_t var_off_seg;

/* K-means. */
static int nprocs;													   /* Clusters spawned.          */
static int dimension;                                                  /* Dimension of data points.  */
static float mindistance;                                              /* Minimum distance.          */
static int ncentroids;                                                 /* Number of centroids.       */
static int lnpoints;                                                   /* Local number of points.    */
static float points[POINTS_SIZE + DELTA*DIMENSION];                    /* Data points.               */
static float centroids[CENTROIDS_SIZE + NUM_CLUSTERS*DELTA*DIMENSION]; /* Data centroids.            */
static int map[MAP_SIZE + DELTA];                                      /* Map of clusters.           */
static int too_far[NUM_CLUSTERS*NUM_THREADS];                          /* Are points too far?        */
static int has_changed[NUM_CLUSTERS*NUM_THREADS];                      /* Has any centroid changed?  */
static int lncentroids[NUM_CLUSTERS];                                  /* Local number of centroids. */
static int ppopulation[PPOPULATION_SIZE + NUM_CLUSTERS*DELTA];         /* Partial population.        */
static float lcentroids[LCENTROIDS_SIZE + DELTA*DIMENSION];            /* Local centroids.           */

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
	off64_t points, centroids;
	off64_t map, too_far, has_changed;
	off64_t lncentroids, ppopulation;			
};

/* Variable offsets auxiliary. */
static struct offsets var_offsets;

static void clone_segments() {
	cloneSegment(&signals_offset_seg, SIG_SEG_0, 0, 0, NULL);
	cloneSegment(&infos_seg, MSG_SEG_0, 0, 0, NULL);
	cloneSegment(&var_off_seg, VAR_OFF_SEG, 0, 0, NULL);
}

static void sync_offsets() {
	/* Send io_signal offset to IO. */
	send_sig_offset();

	mppa_async_malloc(MPPA_ASYNC_DDR_0, (POINTS_SIZE + DELTA*DIMENSION)*sizeof(float), &var_offsets.points, NULL);
	mppa_async_malloc(MPPA_ASYNC_DDR_0, (CENTROIDS_SIZE + NUM_CLUSTERS*DELTA*DIMENSION)*sizeof(float), &var_offsets.centroids, NULL);
	mppa_async_malloc(MPPA_ASYNC_DDR_0, (MAP_SIZE + DELTA)*sizeof(int), &var_offsets.map, NULL);
	mppa_async_malloc(MPPA_ASYNC_DDR_0, (NUM_CLUSTERS*NUM_THREADS)*sizeof(int), &var_offsets.too_far, NULL);
	mppa_async_malloc(MPPA_ASYNC_DDR_0, (NUM_CLUSTERS*NUM_THREADS)*sizeof(int), &var_offsets.has_changed, NULL);
	mppa_async_malloc(MPPA_ASYNC_DDR_0, NUM_CLUSTERS*sizeof(int), &var_offsets.lncentroids, NULL);
	mppa_async_malloc(MPPA_ASYNC_DDR_0, (PPOPULATION_SIZE + NUM_CLUSTERS*DELTA)*sizeof(int), &var_offsets.ppopulation, NULL);

	dataPut(&var_offsets, &var_off_seg, cid, 1, sizeof(struct offsets), NULL);

	send_signal();
}

static void get_work() {
	wait_signal();

	dataGet(lncentroids, MPPA_ASYNC_DDR_0, var_offsets.lncentroids, nprocs, sizeof(int), NULL);

}

int main (__attribute__((unused))int argc, char **argv) {
	/* Initializes async client */
	async_slave_init();

	/* Timer synchronization */
	timer_init();

	/* Util information for the problem. */
	cid = __k1_get_cluster_id();
	nprocs = atoi(argv[0]);
	ncentroids = atoi(argv[1]);
	mindistance = atof(argv[2]);
	dimension = atoi(argv[3]);
	lnpoints = atoi(argv[4]);
	sigback_offset = (off64_t) atoll(argv[5]);
	
	/* Clones message exchange and io_signal segments */
	clone_segments();

	/* Synchronize variables offsets between Master and Slaves. */
	sync_offsets();

	/* Get work from IO. */
	get_work();

	/* Put statistics in stats. segment on IO side. */
	send_statistics(&infos_seg);

	/* Finalizes async library and rpc client */
	async_slave_finalize();

	return 0;
}