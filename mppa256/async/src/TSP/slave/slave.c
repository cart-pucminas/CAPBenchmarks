/* Kernel Includes */
#include <async_util.h>
#include <global.h>
#include <timer.h>
#include <arch.h>
#include <util.h>

/* C And MPPA Library Includes*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Data exchange segments. */
static mppa_async_segment_t infos_seg;

/* TSP. */
int nb_towns;
int seed;

/* Individual Slave statistics. */
uint64_t total = 0;          /* Time spent on slave.    */
uint64_t communication = 0;  /* Time spent on comms.    */               
size_t data_put = 0;         /* Number of bytes put.    */
size_t data_get = 0;         /* Number of bytes gotten. */
unsigned nput = 0;           /* Number of put ops.      */
unsigned nget = 0;           /* Number of get ops.      */

/* Compute Cluster ID & num. of clusters being used. */
int nclusters;
int cid;

/* Timing statistics auxiliars. */
static uint64_t start, end;

static void clone_segments() {
	cloneSegment(&signals_offset_seg, SIG_SEG_0, 0, 0, NULL);
	cloneSegment(&infos_seg, MSG_SEG_0, 0, 0, NULL);
}

void run_tsp (int nb_towns, int seed) {

}

int main(__attribute__((unused))int argc, char **argv) {
	/* Initializes async client */
	async_slave_init();

	/* Timer synchronization */
	timer_init();

	/* Util information for the problem. */
	sigback_offset = (off64_t) atoll(argv[0]);
	nclusters = atoi(argv[1]);
	nb_towns = atoi(argv[2]);
	seed = atoi(argv[3]);
	cid = __k1_get_cluster_id();

	/* Clones message exchange and io_signal segments */
	clone_segments();

	/* Send io_signal offset to IO. */
	send_sig_offset();

	run_tsp(nb_towns, seed);

	/* Finalizes async library and rpc client */
	async_slave_finalize();

	return 0;
}