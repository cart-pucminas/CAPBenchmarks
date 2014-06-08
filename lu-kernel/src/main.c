/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * LU Factorization Benchmark Kernel.
 */

#include <arch.h>
#include <matrix.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <util.h>

#ifndef _MPPA_256_
#include <omp.h>
#endif

/*
 * Problem.
 */
struct problem
{
	int height; /* Matrix height. */
	int width;  /* Matrix width.  */
};

/* Problem sizes. */
static struct problem tiny        =  {  512,  512 };
static struct problem small       =  { 1024, 1024 };
static struct problem workstation =  { 2048, 2048 };
static struct problem standard    =  { 4096, 4096 };
static struct problem large       =  { 8192, 8192 };

/* Benchmark parameters. */
int verbose = 0;                  /* Be verbose?        */
static int seed = 1;              /* Seed value.        */
int nthreads = 1;                 /* Number of threads. */
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
	printf("  --nthreads <value> Set number of threads\n");
	printf("  --class <name>     Set problem class:\n");
	printf("                       - small\n");
	printf("                       - workstation\n");
	printf("                       - standard\n");
	printf("                       - large\n");
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
	#define SET_NTHREADS 1 /* Set number of threads. */
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
					else if (!strcmp(argv[i], "workstation"))
						p = &workstation;
					else if (!strcmp(argv[i], "standard"))
						p = &standard;
					else if (!strcmp(argv[i], "large"))
						p = &large;
					else 
						usage();
					state = READ_ARG;
					break;
				
				/* Set number of threads. */
				case SET_NTHREADS :
					nthreads = atoi(arg);
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
		else if (!strcmp(arg, "--nthreads"))
			state = SET_NTHREADS;
		else if (!strcmp(arg, "--class"))
			state = SET_CLASS;
		else
			usage();
	}
	
	/* Invalid argument(s). */
	if (nthreads < 1)
		usage();
}

/*
 * Runs benchmark.
 */
int main(int argc, char **argv)
{
	matrix_t m;     /* Matrix.       */
	matrix_t l;     /* Lower matrix. */
	matrix_t u;     /* Upper matrix. */
	uint64_t end;   /* End time.     */
	uint64_t start; /* Start time.   */
	
	readargs(argc, argv);
	
	srandnum(seed);
	timer_init();
#ifndef _MPPA_256_	
	omp_set_num_threads(nthreads);
#endif
	
	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");
	start = timer_get();
	m = matrix_create(p->height, p->width);
	l = matrix_create(p->height, p->width);
	u = matrix_create(p->height, p->width);
	matrix_random(m);
	end = timer_get();
	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);
	
	/* Matrix factorization. */
	if (verbose)
		printf("factorizing...\n");
	start = timer_get();
	matrix_lu(m, l, u);
	end = timer_get();
	if (verbose)
		printf("  time spent: ");
	printf("%f\n", timer_diff(start, end)*MICROSEC);
	
	/* House keeping. */
	matrix_destroy(u);
	matrix_destroy(l);
	matrix_destroy(m);
	return (0);
}
