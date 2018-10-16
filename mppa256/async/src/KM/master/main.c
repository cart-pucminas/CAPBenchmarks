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

/* Problem initials and FullName */
char *bench_initials = "KM";
char *bench_fullName = "K-Means";

/* Timing statistics. */
uint64_t master = 0;          /* Time spent on master.        */
uint64_t slave[NUM_CLUSTERS]; /* Time spent on slaves.        */
uint64_t communication = 0;   /* Time spent on communication. */
uint64_t total = 0;           /* Total time.                  */

/* Data exchange statistics. */
size_t data_sent = 0;     /* Number of bytes received. */
unsigned nsend = 0;       /* Number of sends.          */
size_t data_received = 0; /* Number of bytes sent.     */
unsigned nreceive = 0;    /* Number of receives.       */

/* Problem sizes. */
static struct problem tiny     = {  4096, 16,  256, 0.0 };
static struct problem small    = {  8192, 16,  512, 0.0 };
static struct problem standard = { 16384, 16, 1024, 0.0 };
static struct problem large    = { 32768, 16, 1024, 0.0 };
static struct problem huge     = { 65536, 16, 1024, 0.0 };

/* Benchmark parameters. */
int verbose = 0;                  /* Be verbose?        */
static int seed = 0;              /* Seed value.        */
int nclusters = 1;                /* Number of threads. */
static struct problem *p = &tiny; /* Problem.           */

/*
 * Runs benchmark.
 */
int main(int argc, char **argv)
{
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
	data = smalloc(p->npoints*sizeof(vector_t));
	for (i = 0; i < p->npoints; i++) {
		data[i] = vector_create(p->dimension);
		vector_random(data[i]);
	}
	end = timer_get();
	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);
	
	/* Cluster data. */
	if (verbose)
		printf("clustering data...\n");
	start = timer_get();
	map = kmeans(data, p->npoints, p->ncentroids, p->mindistance);
	end = timer_get();
	total = timer_diff(start, end);
	
	/* House keeping. */
	free(map);
	for (i = 0; i < p->npoints; i++)
		vector_destroy(data[i]);
	free(data);
	
	return (0);
}