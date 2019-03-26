/* Kernel Includes */
#include <async_util.h>
#include <global.h>
#include <timer.h>
#include <util.h>
#include "../kernel.h"

/* C And MPPA Library Includes*/
#include <omp.h>
#include <stdio.h>
#include <string.h>

/* FAST parameters. */
static int masksize;
static int mask[MASK_SIZE];
static char chunk[(CHUNK_SIZE*CHUNK_SIZE)+IMG_SIZE*MASK_RADIUS];
static int corners[MAX_THREADS];
static int output[CHUNK_SIZE*CHUNK_SIZE];

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

int main(__attribute__((unused))int argc, char **argv) {
	/* Initializes async client */
	async_slave_init();

	/* Timer synchronization */
	timer_init();

	/* Util information for the problem. */
	cid = __k1_get_cluster_id();

	/* Finalizes async library and rpc client */
	async_slave_finalize();
	
	return 0;
}