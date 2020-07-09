#include <arch.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <timer.h>
#include <util.h>
#include <math.h>
#include "master.h"
#include "matrix.h"

/* Timing statistics. */
uint64_t master = 0;          /* Time spent on master.        */
uint64_t spawn = 0;           /* Time spent spawning slaves   */
uint64_t slave[NUM_CLUSTERS]; /* Time spent on slaves.        */
uint64_t communication = 0;   /* Time spent on communication. */
uint64_t total = 0;           /* Total time.                  */

/* Data exchange statistics. */
size_t data_sent = 0;     /* Number of bytes received. */
unsigned nsend = 0;       /* Number of sends.          */
size_t data_received = 0; /* Number of bytes sent.     */
unsigned nreceive = 0;    /* Number of receives.       */

/*
 * Problem.
 */
struct problem
{
	int height; /* Matrix height. */
	int width;  /* Matrix width.  */
};

/* Problem sizes. */
static struct problem tiny     =  {  512,  512 };
static struct problem small    =  { 1024, 1024 };
static struct problem standard =  { 1536, 1536 };
static struct problem large    =  { 2048, 2048 };
static struct problem huge     =  { 2560, 2560 };

/* Benchmark parameters. */
int verbose = 0;                  /* Be verbose?        */
static int seed = 1;              /* Seed value.        */
int nclusters = 1;                 /* Number of threads. */
static struct problem *p = &tiny; /* Problem.           */

/*
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: lu [options]\n");
	printf("Brief: LU Factorization Benchmark Kernel\n");
	printf("Options:\n");
	printf("  --help             Display this information and exit\n");
	printf("  --nclusters <value> Set number of threads\n");
	printf("  --class <name>     Set problem class:\n");
	printf("                       - small\n");
	printf("                       - standard\n");
	printf("                       - large\n");
	printf("                       - huge\n");
	printf("  --verbose          Be verbose\n");
	exit(0);
}

/*
 * Reads command line arguments.
 */
static void readargs(int argc, char **argv)
{
	int i;     /* Loop index.       */
	char *arg; /* Working argument. */
	int state; /* Processing state. */
	
	/* State values. */
	#define READ_ARG     0 /* Read argument.         */
	#define SET_nclusters 1 /* Set number of threads. */
	#define SET_CLASS    2 /* Set problem class.     */
	
	state = READ_ARG;
	
	/* Read command line arguments. */
	for (i = 1; i < argc; i++)
	{
		arg = argv[i];
		
		/* Set value. */
		if (state != READ_ARG)
		{
			switch (state)
			{
				/* Set problem class. */
				case SET_CLASS :
					if (!strcmp(argv[i], "tiny"))
						p = &tiny;
					else if (!strcmp(argv[i], "small"))
						p = &small;
					else if (!strcmp(argv[i], "standard"))
						p = &standard;
					else if (!strcmp(argv[i], "large"))
						p = &large;
					else if (!strcmp(argv[i], "huge"))
						p = &huge;
					else 
						usage();
					state = READ_ARG;
					break;
				
				/* Set number of threads. */
				case SET_nclusters :
					nclusters = atoi(arg);
					state = READ_ARG;
					break;
				
				default:
					usage();			
			}
			
			continue;
		}
		
		/* Parse argument. */
		if (!strcmp(arg, "--verbose"))
			verbose = 1;
		else if (!strcmp(arg, "--nclusters"))
			state = SET_nclusters;
		else if (!strcmp(arg, "--class"))
			state = SET_CLASS;
		else
			usage();
	}
	
	/* Invalid argument(s). */
	if (nclusters < 1)
		usage();
}

static void inform_statistics() {
	int i;
	
	/* Print timing statistics. */
	printf("timing statistics:\n");
	printf("  master:        %f\n", master*MICROSEC);
	for (i = 0; i < nclusters; i++)
		printf("  slave %d:       %f\n", i, slave[i]*MICROSEC);
	printf("  spawn %d CC:    %f\n", nclusters, spawn*MICROSEC);
	printf("  communication: %f\n", communication*MICROSEC);
	printf("  total time:    %f\n", total*MICROSEC);
	printf("data exchange statistics:\n");
	printf("  data sent:            %d\n", data_sent);
	printf("  number sends:         %u\n", nsend);
	printf("  data received:        %d\n", data_received);
	printf("  number receives:      %u\n", nreceive);
}

static void print_matrix(float *m, int size) {
	int i ,j;
	printf("\n");
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			printf("%.3f, ", ceil(m[(i * size) + j] * 1000) / 1000);
		}
		printf("\n");
	}
	printf("\n");
}

static int check_result(float *m, float *result, int size, int label) {
	int i ,j;
	float rounded_result = 0;
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			rounded_result = ceil(m[(i * size) + j] * 1000) / 1000;
			if (rounded_result != result[(i * size) + j]) {
				if (label == 0) {
					printf("Matriz inferior está errada, deveria ser:");
					print_matrix(result, size);
					return 1;
				} else {
					printf("Matriz superior está errada, deveria ser:");
					print_matrix(result, size);
					return 1;
				}
			}
		}
	}
	return 0;
}

static void test_application(int argc, char **argv) {
	matrix_t l, u;       /* Lower and Upper matrix. */
	matrix_t m;          /* Matrix.                 */

	readargs(argc, argv);

	srandnum(seed);

	printf("initializing...\n");

	m = matrix_create(5, 5);
	l = matrix_create(5, 5);
	u = matrix_create(5, 5);

	float arrayAux[25] = {10, 12, 15, 16, 18,  \
						   8, 9 , 13, 23, 12,  \
					       7, 3 , 2 , 1 , 0 ,  \
				          56, 18, 34, 23, 11,  \
				          45, 13, 11, 10, 10};  

	memcpy(m->elements, arrayAux, 25 * sizeof(float));

	matrix_lu(m, l, u);

	printf("Calculated results: \n\n");

	printf("Matriz inferior:\n");
	print_matrix(l->elements, 5);

	printf("Matriz superior:\n");
	print_matrix(u->elements, 5);

	float lResult[25] = {1.000,  0.000, 0.000, 0.000, 0.000, \
						 0.801,  1.000, 0.000, 0.000, 0.000, \
						 0.700,  9.000, 1.000, 0.000, 0.000, \
						 5.600, 82.000, 7.543, 1.000, 0.000, \
						 4.500, 68.334, 7.134, 0.235, 1.000};

	float uResult[25] = {10.000, 12.000,  15.000,   16.000, 18.000, \
						  0.000, -0.600,   1.000,   10.200, -2.400, \
						  0.000,  0.000, -17.499, -101.999,  9.000, \
						  0.000,  0.000,   0.000, -133.628, 39.115, \
						  0.000,  0.000,   0.000,    0.000, 19.609};

	int error_check = check_result(l->elements, lResult, 5, 0);
	error_check += check_result(u->elements, uResult, 5, 1);

	if (error_check == 0) {
		printf("Matrizes calculadas conforme o esperado!\n\n");
	}

	inform_statistics();

	/* House keeping. */
	matrix_destroy(u);
	matrix_destroy(l);
	matrix_destroy(m);
}

/*
 * Runs benchmark.
 */
int main(int argc, char **argv) {
	// Set this to true if you want to test the application against an already calculated result
	if (false) { 
		test_application(argc, argv);
		return 0;
	}

	uint64_t startTime, endTime; /* Start and End time.     */

	matrix_t l, u;  /* Lower and Upper matrix. */
	matrix_t m;    	/* Matrix.                 */

	int i; /* Loop index. */
	
	readargs(argc, argv);
	
	srandnum(seed);
	timer_init();
	
	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");

	startTime = timer_get();
	m = matrix_create(p->height, p->width);
	l = matrix_create(p->height, p->width);
	u = matrix_create(p->height, p->width);

	matrix_random(m);
	endTime = timer_get();

	if (verbose)
		printf("  time spent: %f\n", timer_diff(startTime, endTime)*MICROSEC);
	
	/* Matrix factorization. */
	if (verbose)
		printf("factorizing...\n");

	startTime = timer_get();

	matrix_lu(m, l, u);
	endTime = timer_get();

	total = timer_diff(startTime, endTime);

	inform_statistics();
	
	/* House keeping. */
	matrix_destroy(u);
	matrix_destroy(l);
	matrix_destroy(m);
	return (0);
}
