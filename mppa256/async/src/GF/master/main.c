/* Kernel Includes */
#include <timer.h>
#include <global.h>
#include <util.h>
#include <problem.h>
#include "../kernel.h"

/* C And MPPA Library Includes*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Gaussian filter. */
extern void gauss_filter (unsigned char *img, int imgsize, double *mask, int masksize);

/* Problem initials and FullName */
char *bench_initials = "GF";
char *bench_fullName = "Gaussian Filter";

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
struct problem tiny	    = {7, 2054}; /* 2048  + (7-1)  = 2054  */
struct problem small    = {7, 4102}; /* 4096  + (7-1)  = 4102  */
struct problem standard = {11, 8202}; /* 8192  + (11-1) = 8202  */
struct problem large    = {11, 16394}; /* 16384 + (11-1) = 16394 */
struct problem huge     = {15, 32768}; /* 32768 + (15-1) = 32782 */

/* Benchmark parameters. */
int verbose = 0;              /* Display informations? */
int nclusters = 1;            /* Number of clusters.   */
static int seed = 0;          /* Seed value.           */
struct problem *prob = &tiny; /* Problem class.        */

/* Generates mask. */
static void generate_mask(double *mask) {
	int half;
	int i, j;
	int masksize;
	double sec;
	double first;
	double total_aux;

	masksize = prob->masksize;
	
	first = 1.0/(2.0*PI*SD*SD);
	half = masksize >> 1;
	total_aux = 0;

	for (i = -half; i <= half; i++) {
		for (j = -half; j <= half; j++) {
			sec = -((i*i + j*j)/2.0*SD*SD);
			sec = pow(E, sec);

			MASK(i + half, j + half) = first*sec;
			total_aux += MASK(i + half, j + half);
		}
	}
	
	for (i = 0 ; i < masksize; i++) {
		for (j = 0; j < masksize; j++)
			MASK(i, j) /= total_aux;
	}
}

/* Runs benchmark. */
int main(int argc, char **argv) {
	double *mask;        /* Mask.              */
	unsigned char *img;  /* Image.             */
	uint64_t start, end; /* Start & End times. */

	readargs(argc, argv);
	timer_init();
	srandnum(seed);

	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");

	start = timer_get();
	img = smalloc(prob->imgsize*prob->imgsize*sizeof(char));

	for (int i = 0; i < prob->imgsize*prob->imgsize; i++)
		img[i] = randnum() & 0xff;

	mask = smalloc(prob->masksize*prob->masksize*sizeof(double));
	generate_mask(mask);
	end = timer_get();

	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);

	/* Apply filter. */
	if (verbose)
		printf("applying filter...\n");

	start = timer_get();
	gauss_filter(img, prob->imgsize, mask, prob->masksize);
	end = timer_get();	

	total = timer_diff(start, end);

	inform_statistics();

	free(mask);
	free(img);
	
	return 0;
}