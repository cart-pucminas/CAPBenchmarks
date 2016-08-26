/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * Friendly Numbers Benchmark Kernel.
 */

#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>
#include <m5op.h>
#include "fn.h"

/*
 * Computes friendly numbers.
 */
extern int friendly_numbers(int start, int end);
extern void m5_reset_stats(uint64_t ns_delay, uint64_t ns_period);
extern void m5_dump_stats(uint64_t ns_delay, uint64_t ns_period);


/*
 * Problem.
 */
struct problem
{
	int start; /* Range start. */
	int end;   /* Range end.   */
};

/* Problem sizes. */
static struct problem tiny     =  {   10001,   14096 };
static struct problem small    =  {  100001,  116384 };
static struct problem standard =  {  500001,  532768 };
static struct problem large    =  { 2000001, 2032768 };
static struct problem huge     =  { 8000001, 8065536 };

/* Be verbose? */
int verbose = 0;

/* Number of threads. */                
int nthreads = 1;

/* Problem. */           
static struct problem *p = &tiny; 

/*
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: fn [options]\n");
	printf("Brief: Friendly Numbers Benchmark Kernel\n");
	printf("Options:\n");
	printf("  --help             Display this information and exit\n");
	printf("  --nthreads <value> Set number of threads\n");
	printf("  --class <name>     Set problem class:\n");
	printf("                       - tiny\n");
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
	uint64_t end;   /* End time.   */
	uint64_t start; /* Start time. */
	
	readargs(argc, argv);
	printf("CAPBench - FN kernel\n");
	printf("  # of threads: %d \n", nthreads);
	
	omp_set_num_threads(nthreads);
	
	/* Compute friendly numbers. */
	printf("Entering in FN ROI - Computing Friendly Numbers...\n");
	start = timer_get();
	m5_reset_stats(0,0);
	friendly_numbers(p->start, p->end);
	end = timer_get();
	m5_dump_stats(0,0);
	
	printf("FN total time:    %f\n", timer_diff(start, end)*MICROSEC);

	return (0);
}
