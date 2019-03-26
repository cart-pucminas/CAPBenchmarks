/* Kernel Includes */
#include <problem.h>
#include <global.h>
#include <timer.h>
#include <util.h>
#include "../kernel.h"

/* C And MPPA Library Includes*/
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* FAST corner detection. */
extern int fast (char *img, char *output,int imgsize, int *mask, int masksize);

/* Problem initials and FullName */
char *bench_initials = "FAST";
char *bench_fullName = "Features from accelerated segment test";

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
struct problem tiny     = {2,27,54,2048};
struct problem small    = {2,27,54,4096};
struct problem standard = {2,27,54,8192};
struct problem large    = {2,27,54,16384};
struct problem huge     = {2,27,54,24576};

/* Benchmark parameters. */
int verbose = 0;                  /* Be verbose?        */
int nclusters = 1;                /* Number of threads. */
static int seed = 0;              /* Seed number.       */
struct problem *prob = &tiny; /* Problem.           */

/* Generates mask. */
static void generate_mask(int *mask) {
	mask[0*prob->maskcolumns + 0] = -1;
	mask[0*prob->maskcolumns + 1] = -3;

	mask[1*prob->maskcolumns + 0] = 0;
	mask[1*prob->maskcolumns + 1] = -3;

	mask[2*prob->maskcolumns + 0] = 1;
	mask[2*prob->maskcolumns + 1] = -3;
	
	mask[3*prob->maskcolumns + 0] = 2;
	mask[3*prob->maskcolumns + 1] = -2;

	mask[4*prob->maskcolumns + 0] = 3;
	mask[4*prob->maskcolumns + 1] = -1;

	mask[5*prob->maskcolumns + 0] = 3;
	mask[5*prob->maskcolumns + 1] = 0;

	mask[6*prob->maskcolumns + 0] = 3;
	mask[6*prob->maskcolumns + 1] = 1;

	mask[7*prob->maskcolumns + 0] = 2;
	mask[7*prob->maskcolumns + 1] = 2;

	mask[8*prob->maskcolumns + 0] = 1;
	mask[8*prob->maskcolumns + 1] = 3;

	mask[9*prob->maskcolumns + 0] = 0;
	mask[9*prob->maskcolumns + 1] = 3;

	mask[10*prob->maskcolumns + 0] = -1;
	mask[10*prob->maskcolumns + 1] = 3;

	mask[11*prob->maskcolumns + 0] = -2;
	mask[11*prob->maskcolumns + 1] = 2;

	mask[12*prob->maskcolumns + 0] = -3;
	mask[12*prob->maskcolumns + 1] = 1;

	mask[13*prob->maskcolumns + 0] = -3;
	mask[13*prob->maskcolumns + 1] = 0;

	mask[14*prob->maskcolumns + 0] = -3;
	mask[14*prob->maskcolumns + 1] = -1;

	mask[15*prob->maskcolumns + 0] = -2;
	mask[15*prob->maskcolumns + 1] = -2;

	mask[16*prob->maskcolumns + 0] = -1;
	mask[16*prob->maskcolumns + 1] = -3;

	mask[17*prob->maskcolumns + 0] = 0;
	mask[17*prob->maskcolumns + 1] = -3;

	mask[18*prob->maskcolumns + 0] = 1;
	mask[18*prob->maskcolumns + 1] = -3;
	
	mask[19*prob->maskcolumns + 0] = 2;
	mask[19*prob->maskcolumns + 1] = -2;

	mask[20*prob->maskcolumns + 0] = 3;
	mask[20*prob->maskcolumns + 1] = -1;

	mask[21*prob->maskcolumns + 0] = 3;
	mask[21*prob->maskcolumns + 1] = 0;

	mask[22*prob->maskcolumns + 0] = 3;
	mask[22*prob->maskcolumns + 1] = 1;

	mask[23*prob->maskcolumns + 0] = 2;
	mask[23*prob->maskcolumns + 1] = 2;
	
	mask[24*prob->maskcolumns + 0] = 1;
	mask[24*prob->maskcolumns + 1] = 3;

	mask[25*prob->maskcolumns + 0] = 0;
	mask[25*prob->maskcolumns + 1] = 3;

	mask[26*prob->maskcolumns + 0] = -1;
	mask[26*prob->maskcolumns + 1] = 3;
}

/* Runs benchmark. */
int main(int argc, char **argv) {
	int *mask;      	 /* Mask.                  */
	char *img; 			 /* Image input.           */
	char *output;		 /* Image output.		   */
	int numcorners=0;	 /* Total corners detected */
	uint64_t start, end; /* Start & End times.     */

	readargs(argc, argv);
	timer_init();
	srandnum(seed);
	
	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");

	start = timer_get();
	img = smalloc(prob->imgsize*prob->imgsize*sizeof(char));
	output = scalloc(prob->imgsize*prob->imgsize,sizeof(char));

	for (int i = 0; i < prob->imgsize*prob->imgsize; i++){
		char val = (char) (randnum() & 0xff);
		img[i] = (val>0) ? val : val*(-1);
	}

	mask = smalloc(prob->maskrows*prob->maskcolumns*sizeof(int));
	generate_mask(mask);
	end = timer_get();

	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);
		
	/* Apply filter. */
	if (verbose)
		printf("Detecting corners...\n");

	start = timer_get();
	numcorners = fast(img, output, prob->imgsize, mask, prob->masksize);
	end = timer_get();
	
	total = timer_diff(start, end);

	inform_statistics();
	
	/* House keeping. */
	free(mask);
	free(img);
	free(output);
	
	return (0);
}