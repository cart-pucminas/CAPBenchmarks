/* Kernel Includes */
#include <arch.h>
#include <util.h>
#include <timer.h>
#include <global.h>
#include <problem.h>
#include "vector.h"

/* C And MPPA Library Includes*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Clusters data. */
extern int *kmeans(vector_t *_points, int _npoints, int _ncentroids, float _mindistance);

/* Problem initials and FullName */
char *bench_initials = "KM";
char *bench_fullName = "K-Means";

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
struct problem tiny     = {  4096, 16,  256, 0.0 };
struct problem small    = {  8192, 16,  512, 0.0 };
struct problem standard = { 16384, 16, 1024, 0.0 };
struct problem large    = { 32768, 16, 1024, 0.0 };
struct problem huge     = { 65536, 16, 1024, 0.0 };

/* Benchmark parameters. */
int verbose = 0;              /* Display informations? */
int nclusters = 1;            /* Number of clusters.   */
static int seed = 0;          /* Seed value.           */
struct problem *prob = &tiny; /* Problem class.        */

/*
 * Runs benchmark.
 */
int main(int argc, char **argv) {
	int i;          /* Loop index.      */
	int *map;       /* Map of clusters. */
	uint64_t end;   /* End time.        */
	uint64_t start; /* Start time.      */
	vector_t *data; /* Data points.     */
	
	readargs(argc, argv);
	
	timer_init();
	srandnum(seed);
	
	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");

	start = timer_get();
	data = smalloc(prob->npoints*sizeof(vector_t));
	for (i = 0; i < prob->npoints; i++) {
		data[i] = vector_create(prob->dimension);
		vector_random(data[i]);
	}
	end = timer_get();

	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);
	
	/* Cluster data. */
	if (verbose)
		printf("clustering data...\n");

	start = timer_get();
	map = kmeans(data, prob->npoints, prob->ncentroids, prob->mindistance);
	end = timer_get();
	total = timer_diff(start, end);

	inform_statistics();
	
	/* House keeping. */
	free(map);
	for (i = 0; i < prob->npoints; i++)
		vector_destroy(data[i]);
	free(data);
	
	return (0);
}