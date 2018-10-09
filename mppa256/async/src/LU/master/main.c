/* Kernel Include */
#include <util.h>
#include <timer.h>
#include <global.h>
#include <problem.h>
#include "matrix.h"
#include "master.h"

/* C And MPPA Library Includes*/
#include <stdio.h>

/* Problem initials and FullName */
char *bench_initials = "LU";
char *bench_fullName = "LU Factorization";

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
struct problem tiny     =  {  512,  512 };
struct problem small    =  { 1024, 1024 };
struct problem standard =  { 1536, 1536 };
struct problem large    =  { 2048, 2048 };
struct problem huge     =  { 2560, 2560 };

/* Statistics information from clusters */
mppa_async_segment_t infos_segment;
Info infos[NUM_CLUSTERS];

/* Benchmark parameters. */
int verbose = 0;              /* Display informations? */
int nclusters = 1;            /* Number of clusters.   */
static int seed = 1;          /* Seed value.           */
struct problem *prob = &tiny; /* Problem class.        */

static void setAllStatistics() {
	uint64_t comm_Sum = 0;
	uint64_t comm_Average = 0;
	for (int i = 0; i < nclusters; i++) {
		slave[i] = infos[i].slave;
		comm_Sum += infos[i].communication;
		data_put += infos[i].data_put;
		data_get += infos[i].data_get;
		nput += infos[i].nput;
		nget += infos[i].nget;
	}

	comm_Average = (uint64_t)comm_Sum/nclusters;
	communication += comm_Average;
}

int main(int argc, char **argv) {
	uint64_t startTime, endTime; /* Start and End time.     */

	matrix_t l, u;       /* Lower and Upper matrix. */
	matrix_t m;          /* Matrix.                 */
	
	readargs(argc, argv);
	
	srandnum(seed);
	timer_init();
	
	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");

	/*
	startTime = timer_get();
	m = matrix_create(prob->height, prob->width);
	l = matrix_create(prob->height, prob->width);
	u = matrix_create(prob->height, prob->width);

	matrix_random(m);
	endTime = timer_get();*/

	if (verbose)
		printf("  time spent: %f\n", timer_diff(startTime, endTime)*MICROSEC);
	
	/* Matrix factorization. */
	if (verbose)
		printf("factorizing...\n");


	int oi = 3;
	m = matrix_create(oi,oi);
	l = matrix_create(oi,oi);
	u = matrix_create(oi,oi);

	MATRIX(m, 0, 0) = 2;
	MATRIX(m, 0, 1) = 3;
	MATRIX(m, 0, 2) = -2;
	MATRIX(m, 1, 0) = -4;
	MATRIX(m, 1, 1) = 3;
	MATRIX(m, 1, 2) = 3;
	MATRIX(m, 2, 0) = -4;
	MATRIX(m, 2, 1) = -2;
	MATRIX(m, 2, 2) = 8;

	startTime = timer_get();
	matrix_lu(m, l, u);
	endTime = timer_get();

	total = timer_diff(startTime, endTime);

	for (int i = 0; i < oi; i++) {
		for (int j = 0; j < oi; j++) 
			printf("|| %.2f ", l->elements[(i*oi)+j]);
		printf("\n");
	}

	fflush(stdout);
	printf("\n");

	for (int i = 0; i < oi; i++) {
		for (int j = 0; j < oi; j++) 
			printf("|| %.2f ", u->elements[(i*oi)+j]);
		printf("\n");
	}

	fflush(stdout);

	/* Sets all statistics from slaves. */
	setAllStatistics();

	inform_statistics();
	
	/* House keeping. */
	matrix_destroy(u);
	matrix_destroy(l);
	matrix_destroy(m);
	return 0;
}