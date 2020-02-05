/* Kernel Includes */
#include <problem.h>
#include <global.h>

/* C And MPPA Library Includes*/
#include <stdint.h>
#include <stdlib.h>

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
static int seed = 122;          /* Seed number.       */
struct problem *prob = &tiny; /* Problem.           */

/*
 * Runs benchmark.
 */
int main (int argc, char **argv) {

	readargs(argc, argv);

	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");

	return 0;
}