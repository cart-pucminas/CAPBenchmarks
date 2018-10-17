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

/* K-means. */
int dimension;
static int nprocs;
static float mindistance;                                              /* Minimum distance.          */
static int ncentroids;                                                 /* Number of centroids.       */
static float points[POINTS_SIZE + DELTA*DIMENSION];                    /* Data points.               */
static float centroids[CENTROIDS_SIZE + NUM_CLUSTERS*DELTA*DIMENSION]; /* Data centroids.            */
static int map[MAP_SIZE + DELTA];                                      /* Map of clusters.           */
static int too_far[NUM_CLUSTERS*NUM_THREADS];                          /* Are points too far?        */
static int has_changed[NUM_CLUSTERS*NUM_THREADS];                      /* Hash any centroid changed? */
static int lncentroids[NUM_CLUSTERS];                                  /* Local number of centroids. */
static int lnpoints;                                                   /* Local number of points.    */
static int ppopulation[PPOPULATION_SIZE + NUM_CLUSTERS*DELTA];         /* Partial population.        */
static float lcentroids[LCENTROIDS_SIZE + DELTA*DIMENSION];            /* Local centroids.           */

/* Thread communication. */
static omp_lock_t lock[NUM_THREADS];

/* Individual Slave statistics. */
uint64_t total = 0;          /* Time spent on slave.    */
uint64_t communication = 0;  /* Time spent on comms.    */               
size_t data_put = 0;         /* Number of bytes put.    */
size_t data_get = 0;         /* Number of bytes gotten. */
unsigned nput = 0;           /* Number of put ops.      */
unsigned nget = 0;           /* Number of get ops.      */

/* Timing statistics auxiliars. */
static uint64_t start, end;

/* Compute Cluster ID */
int cid;

static void clone_segments() {
	cloneSegment(&signals_offset_seg, SIG_SEG_0, 0, 0, NULL);
	cloneSegment(&infos_seg, MSG_SEG_0, 0, 0, NULL);
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

	/* Send io_signal offset to IO. */
	send_sig_offset();

	/* Get work from IO. */
	//get_work();

	/* Put statistics in stats. segment on IO side. */
	send_statistics(&infos_seg);

	/* Finalizes async library and rpc client */
	async_slave_finalize();

	return 0;
}