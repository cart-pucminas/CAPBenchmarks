/* Kernel Includes */
#include <problem.h>
#include <global.h>
#include <timer.h>
#include <util.h>

/* C And MPPA Library Includes*/
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Bucket sort algorithm. */
extern void bucketsort(int *array, int n);

/* Problem initials and FullName */
char *bench_initials = "IS";
char *bench_fullName = "Integer Sort";

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
struct problem tiny     = {   8388608 };
struct problem small    = {  16777216 };
struct problem standard = {  33554432 };
struct problem large    = {  67108864 };
struct problem huge     = { 134217728 };

/* Benchmark parameters. */
int verbose = 0;              /* Be verbose?        */
int nclusters = 1;            /* Number of threads. */
static int seed = 0;          /* Seed number.       */
struct problem *prob = &tiny; /* Problem.           */

/* Runs benchmark. */
int main(int argc, char **argv) {
	int i;               /* Loop index.         */
	int *a;              /* Array to be sorted. */
	uint64_t start, end; /* Start & End times.  */

	readargs(argc, argv);
	timer_init();
	srandnum(seed);

	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");

	start = timer_get();
	a = smalloc(prob->n*sizeof(int));

	for (i = 0; i < prob->n; i++)
		a[i] = randnum() & 0xfffff;
	end = timer_get();

	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);

	/* Cluster data. */
	if (verbose)
		printf("sorting...\n");

	/* Sorting... */
	start = timer_get();
	bucketsort(a, prob->n);
	end = timer_get();

	total = timer_diff(start, end);

	inform_statistics();

	/* House keeping. */
	free(a);

	return (0);
}