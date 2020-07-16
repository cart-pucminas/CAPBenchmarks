/* Kernel Includes */
#include <arch.h>
#include <util.h>
#include <timer.h>
#include <global.h>
#include <problem.h>

/* C And MPPA Library Includes*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Clusters data. */
extern int *kmeans(float *_points, int _npoints, int _ncentroids, int _dimension);

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
struct problem tiny     = {  4096, 256};
struct problem small    = {  8192, 512};
struct problem standard = { 16384, 1024};
struct problem large    = { 32768, 1024};
struct problem huge     = { 65536, 1024};

/* Benchmark parameters. */
int verbose = 0;              /* Display informations? */
int nclusters = 1;            /* Number of clusters.   */
static int seed = 0;          /* Seed value.           */
struct problem *prob = &tiny; /* Problem class.        */

/* Runs benchmark. */
int main(int argc, char **argv) {
	int i;               /* Loop index.      */
	int *map;            /* Map of clusters. */
	int dimension;       /* Dimension of points. */
	float *points;       /* Data points.     */
	uint64_t start, end; /* End time.        */
	
	readargs(argc, argv);
	
	timer_init();
	srandnum(seed);

	/* Setting the dimension for the problem. */
	dimension = 16; 
	
	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");
	start = timer_get();
	points = smalloc(prob->npoints * dimension * sizeof(float));
	for (i = 0; i < prob->npoints * dimension; i++)
		points[i] = randnum() & 0xffff;
	end = timer_get();

	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);
	
	/* Cluster data. */
	if (verbose)
		printf("clustering data...\n");
	start = timer_get();
	map = kmeans(points, prob->npoints, prob->ncentroids, dimension);
	end = timer_get();
	total = timer_diff(start, end);

	inform_statistics();
	
	/* House keeping. */
	free(map);
	free(points);
	
	return (0);
}