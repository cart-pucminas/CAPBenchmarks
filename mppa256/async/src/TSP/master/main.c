/* Kernel Includes */
#include <problem.h>
#include <global.h>
#include "../common_main.h"

/* C And MPPA Library Includes*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Problem initials and FullName */
char *bench_initials = "TSP";
char *bench_fullName = "Traveler Salesman Problem";

/* Timing statistics. */
uint64_t master = 0;          /* Time spent on master.        */
uint64_t spawn = 0;           /* Time spent spawning slaves   */
uint64_t slave[NUM_CLUSTERS]; /* Time spent on slaves.        */
uint64_t communication = 0;   /* Time spent on communication. */
uint64_t total = 0;           /* Total time.                  */

/* Data exchange statistics. */
size_t data_put = 0; /* Number of bytes put.    */
unsigned nput = 0;   /* Number of bytes gotten. */
size_t data_get = 0; /* Number of items put.    */
unsigned nget = 0;   /* Number of items gotten. */

/* Problem sizes. */
struct problem tiny     =  { 14 };
struct problem small    =  { 15 };
struct problem standard =  { 17 };
struct problem large    =  { 19 };
struct problem huge     =  { 20 };

/* Benchmark parameters. */
int verbose = 0;              /* Be verbose?        */
int nclusters = 1;            /* Number of threads. */
static int seed = 122;        /* Seed number.     */
struct problem *prob = &tiny; /* Problem.           */

int parametersOk() {
    int entries = queue_size(nclusters, prob->nb_towns, NULL);
    int req_mem = sizeof(job_queue_node_t) * entries;
    if (req_mem > MAX_MEM_PER_CLUSTER) {
        printf("Error, not enough memory. Verify MAX_TOWNS (%d), MIN_JOBS_THREAD (%d), and MAX_MEM_PER_CLUSTER (%d) parameters. Requested memory: %d bytes (should be < %d bytes)\n",
	       MAX_TOWNS, MIN_JOBS_THREAD, MAX_MEM_PER_CLUSTER, req_mem, MAX_MEM_PER_CLUSTER);
        return 0;
    }
    return 1;
}

/*
 * Runs benchmark.
 */
int main (int argc, char **argv) {

	readargs(argc, argv);

	if (parametersOk()) {
        /* Always run with 16 threads per cluster by default */
        run_tsp(prob->nb_towns, nclusters);
    } else {
        printf("Invalid parameters. Terminating execution.\n");
    }

	return 0;
}

void run_tsp (int nb_towns, int nb_clusters) {

	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");
}