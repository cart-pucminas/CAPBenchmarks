/* Kernel Includes */
#include <async_util.h>
#include <timer.h>
#include <arch.h>
#include <util.h>
#include "slave.h"

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

/* K-means. */
int dimension;                                                         /* Dimension of data points.  */
static int nprocs;													   /* Clusters spawned.          */
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
	int points, centroids;
	int map, too_far, has_changed;
	int lncentroids, ppopulation, lcentroids;			
};

/* Variable offsets auxiliary. */
static struct offsets var_offsets;

/*============================================================================*
 *                                populate()                                 *
 *============================================================================*/

int first = 1;
/* Populates clusters. */
static void populate() {
	float tmp;      /* Auxiliary variable. */
	float distance; /* Smallest distance.  */

	start = timer_get();
	memset(&too_far[cid*NUM_THREADS], 0, NUM_THREADS*sizeof(int)); 
	
	/* Iterate over data points. */
	#pragma omp parallel for schedule(static) default(shared) private(tmp, distance)
	for (int i = 0; i < lnpoints; i++) {
		distance = vector_distance(CENTROID(map[i]), POINT(i));
		
		/* Look for closest cluster. */
		for (int j = 0; j < ncentroids; j++) {
			/* Point is in this cluster. */
			if (j == map[i])
				continue;
				
			tmp = vector_distance(CENTROID(j), POINT(i));
			
			/* Found. */
			if (tmp < distance) {
				map[i] = j;
				distance = tmp;
			}
		}

		if (first) {
			printf("%d\n", map[i]);
			fflush(stdout);
		}

		/* Cluster is too far away. */
		if (distance > mindistance)
			too_far[cid*NUM_THREADS + omp_get_thread_num()] = 1;
	}

	first = 0;

	end = timer_get();
	total += timer_diff(start, end);
}

/*============================================================================*
 *                          compute_centroids()                               *
 *============================================================================*/

/* Synchronizes partial centroids. */
static void sync_pcentroids() {	
	/* Send partial centroids. */
	dataPut(centroids, MPPA_ASYNC_DDR_0, var_offsets.centroids, ncentroids*dimension, sizeof(float), NULL);
	send_signal();

	/* Receive partial centroids. */
	wait_signal();
	dataGet(centroids, MPPA_ASYNC_DDR_0, var_offsets.centroids, nprocs*lncentroids[cid]*dimension, sizeof(float), NULL);
}

/* Synchronizes partial population. */
static void sync_ppopulation() {
	/* Send partial population. */
	dataPut(ppopulation, MPPA_ASYNC_DDR_0, var_offsets.ppopulation, ncentroids, sizeof(int), NULL);
	send_signal();

	/* Receive partial population. */
	wait_signal();
	dataGet(ppopulation, MPPA_ASYNC_DDR_0, var_offsets.ppopulation, nprocs*lncentroids[cid], sizeof(int), NULL);
}

/* Synchronizes centroids. */
static void sync_centroids() {
	dataPut(lcentroids, MPPA_ASYNC_DDR_0, var_offsets.lcentroids, lncentroids[cid]*dimension, sizeof(float), NULL);
	send_signal();

	wait_signal();
	dataGet(centroids, MPPA_ASYNC_DDR_0, var_offsets.centroids, ncentroids*dimension, sizeof(float), NULL);
}

/* Synchronizes status. */
static void sync_status() {
	dataPut(&has_changed[cid*NUM_THREADS], MPPA_ASYNC_DDR_0, var_offsets.has_changed, NUM_THREADS, sizeof(int), NULL);
	dataPut(&too_far[cid*NUM_THREADS], MPPA_ASYNC_DDR_0, var_offsets.too_far, NUM_THREADS, sizeof(int), NULL);
	send_signal();

	wait_signal();
	dataGet(has_changed, MPPA_ASYNC_DDR_0, var_offsets.has_changed, nprocs*NUM_THREADS, sizeof(int), NULL);
	dataGet(too_far, MPPA_ASYNC_DDR_0, var_offsets.too_far, nprocs*NUM_THREADS, sizeof(int), NULL);
}

/* Computes clusters' centroids. */
static void compute_centroids() {
	int i, j;       /* Loop indexes.        */
	int population; /* Centroid population. */

	start = timer_get();
	
	memcpy(lcentroids, CENTROID(cid*(ncentroids/nprocs)), lncentroids[cid]*dimension*sizeof(float));
	memset(&has_changed[cid*NUM_THREADS], 0, NUM_THREADS*sizeof(int));
	memset(centroids, 0, (ncentroids + DELTA*nprocs)*dimension*sizeof(float));
	memset(ppopulation, 0, (ncentroids + nprocs*DELTA)*sizeof(int));

	/* Compute partial centroids. */
	#pragma omp parallel for schedule(static) default(shared) private(i, j)
	for (i = 0; i < lnpoints; i++) {
		j = map[i]%NUM_THREADS;
		
		omp_set_lock(&lock[j]);
		
		vector_add(CENTROID(map[i]), POINT(i));
			
		ppopulation[map[i]]++;
		
		omp_unset_lock(&lock[j]);
	}

	end = timer_get();
	total += timer_diff(start, end);

	sync_pcentroids();
	sync_ppopulation();

	start = timer_get();

	/* Compute centroids. */
	#pragma omp parallel for schedule(static) default(shared) private(i, j, population)
	for (j = 0; j < lncentroids[cid]; j++) {
		population = 0;
		
		for (i = 0; i < nprocs; i++) {
			if (*POPULATION(i, j) == 0)
				continue;
			
			population += *POPULATION(i, j);
			
			if (i == cid)
				continue;
			
			vector_add(PCENTROID(cid, j), PCENTROID(i, j));
		}
		
		if (population > 1)
			vector_mult(PCENTROID(cid, j), 1.0/population);
		
		/* Cluster mean has changed. */
		if (!vector_equal(PCENTROID(cid, j), LCENTROID(j))) {
			has_changed[cid*NUM_THREADS + omp_get_thread_num()] = 1;
			vector_assign(LCENTROID(j), PCENTROID(cid, j));
		}
	}
	
	end = timer_get();
	total += timer_diff(start, end);

	sync_centroids();
	sync_status();
}

/*============================================================================*
 *                                 again()                                    *
 *============================================================================*/

/* Asserts if another iteration is needed. */
static int again() {
	start = timer_get();
	/* Checks if another iteration is needed. */	
	for (int i = 0; i < nprocs*NUM_THREADS; i++) {
		if (has_changed[i] && too_far[i]) {
			end = timer_get();
			total += timer_diff(start, end);
			return (1);
		}
	}
	
	end = timer_get();
	total += timer_diff(start, end);

	return (0);
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
	} while (again());
}

/*============================================================================*
 *                                 getwork()                                  *
 *============================================================================*/

static void clone_segments() {
	cloneSegment(&signals_offset_seg, SIG_SEG_0, 0, 0, NULL);
	cloneSegment(&infos_seg, MSG_SEG_0, 0, 0, NULL);
	cloneSegment(&var_off_seg, VAR_OFF_SEG, 0, 0, NULL);
}

static void sync_offsets() {

	/* Offset in bytes. */
	off64_t points_aux, centroids_aux;
	off64_t map_aux, too_far_aux, has_changed_aux;
	off64_t lncentroids_aux, ppopulation_aux, lcentroids_aux; 

	async_smalloc(MPPA_ASYNC_DDR_0, (POINTS_SIZE + DELTA*DIMENSION)*sizeof(float), &points_aux, NULL);
	async_smalloc(MPPA_ASYNC_DDR_0, (CENTROIDS_SIZE + NUM_CLUSTERS*DELTA*DIMENSION)*sizeof(float), &centroids_aux, NULL);
	async_smalloc(MPPA_ASYNC_DDR_0, (MAP_SIZE + DELTA)*sizeof(int), &map_aux, NULL);
	async_smalloc(MPPA_ASYNC_DDR_0, (NUM_CLUSTERS*NUM_THREADS)*sizeof(int), &too_far_aux, NULL);
	async_smalloc(MPPA_ASYNC_DDR_0, (NUM_CLUSTERS*NUM_THREADS)*sizeof(int), &has_changed_aux, NULL);
	async_smalloc(MPPA_ASYNC_DDR_0, NUM_CLUSTERS*sizeof(int), &lncentroids_aux, NULL);
	async_smalloc(MPPA_ASYNC_DDR_0, (PPOPULATION_SIZE + NUM_CLUSTERS*DELTA)*sizeof(int), &ppopulation_aux, NULL);
	async_smalloc(MPPA_ASYNC_DDR_0, (LCENTROIDS_SIZE + DELTA*DIMENSION)*sizeof(float), &lcentroids_aux, NULL);

	/* Converting offsets to positions. */
	var_offsets.points = points_aux/sizeof(float);
	var_offsets.centroids = centroids_aux/sizeof(float);
	var_offsets.map = map_aux/sizeof(int);
	var_offsets.too_far = too_far_aux/sizeof(int);
	var_offsets.has_changed = has_changed_aux/sizeof(int);
	var_offsets.lncentroids = lncentroids_aux/sizeof(int);
	var_offsets.ppopulation = ppopulation_aux/sizeof(int);
	var_offsets.lcentroids = lcentroids_aux/sizeof(float);

	dataPut(&var_offsets, &var_off_seg, cid, 1, sizeof(struct offsets), NULL);

	/* Send io_signal offset to IO. */
	send_sig_offset();
}

/* Receives work from master process. */
static void get_work() {
	wait_signal();

	dataGet(lncentroids, MPPA_ASYNC_DDR_0, var_offsets.lncentroids, nprocs, sizeof(int), NULL);

	for (int i = 0; i < lnpoints; i++)
		dataGet(&points[i*dimension], MPPA_ASYNC_DDR_0, var_offsets.points, dimension, sizeof(float), NULL);

	dataGet(centroids, MPPA_ASYNC_DDR_0, var_offsets.centroids, ncentroids*dimension, sizeof(float), NULL);

	dataGet(map, MPPA_ASYNC_DDR_0, var_offsets.map, lnpoints, sizeof(int), NULL);
}

/*============================================================================*
 *                                  main()                                    *
 *============================================================================*/

static void send_result() {
	dataPut(map, MPPA_ASYNC_DDR_0, var_offsets.map, lnpoints, sizeof(int), NULL);

	/* Data is ready signal. */
	send_signal();

	/* Handshake before statistics exchange. */
	wait_signal();

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

	/* Start of km solving. */
	kmeans();

	/* Sends mapping result and statistics to IO. */
	send_result();

	/* Finalizes async library and rpc client */
	async_slave_finalize();

	return 0;
}