#include <infos.h>
#include <global.h>
#include <timer.h>
#include <stdio.h>
#include <stdlib.h>

#include "master.h"

/* Definition of "String" variables for infos.c */
char *bench_initials = "FN";
char *bench_fullName = "Friendly Numbers Benchmark Kernel";

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

/* Problem. */
struct problem {
	int start; // Initial number of interval
	int end;   // Final number of invertal
};

static struct problem tiny     =  { 8000001, 8004096 };
static struct problem small    =  { 8000001, 8008192 };
static struct problem standard =  { 8000001, 8016384 };
static struct problem large    =  { 8000001, 8032768 };
static struct problem huge     =  { 8000001, 8065536 }; 

/* Benchmark parameters. */
int verbose = 0;   /* Display informations? 	   */
int nclusters = 1; /* Quantity of Clusters spawned */
int npes = 1; /* Number of threads on each cluster */
static struct problem *p = &tiny; /* Problem Class */

static void readargs(int argc, char **argv) {
	int i;     /* Loop index.       */
	char *arg; /* Working argument. */
	int state; /* Processing state. */
	
	/* State values. */
	#define READ_ARG     0 /* Read argument.         */
	#define SET_nclusters 1 /* Set number of clusters. */
	#define SET_CLASS    2 /* Set problem class.     */
	#define SET_threads 3 /* Set number of threads */
	
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
						inform_usage();
					state = READ_ARG;
					break;
				
				/* Set number of threads. */
				case SET_nclusters :
					nclusters = atoi(arg);
					state = READ_ARG;
					break;
				
				case SET_threads:
					npes = atoi(arg);
					state = READ_ARG;
					break;

				default:
					inform_usage();		
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
		else if (!strcmp(arg, "--nthreads"))
			state = SET_threads;
		else
			inform_usage();
	}
	
	/* Invalid argument(s). */
	if (nclusters < 1)
		inform_usage();
}

int main(int argc, const char **argv) {
	uint64_t startTime, endTime;

	timer_init();

	readargs(argc, argv);

	inform_actual_benchmark();

	startTime = timer_get();
	friendly_numbers(p->start, p->end);
	endTime = timer_get();
	total = timer_diff(startTime, endTime);

	inform_statistics();
	
	return 0;
}