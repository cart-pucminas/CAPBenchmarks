/*
 * Copyright(C) 2016 Matheus Alcântara Souza <matheusalcantarasouza@gmail.com>
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <timer.h>
#include <util.h>
#include "nbody.h"

/*
 * Problem.
 */
struct problem
{
	int nbodies;    /* Number of bodies or particles. O(n^2) */
	int niter;      /* Number of iterations. O(n) */
};

/* Problem sizes. */
static struct problem tiny     =  { 8192 , 24 };
static struct problem small    =  { 16384, 12 };
static struct problem standard =  { 16384, 24 };
static struct problem large    =  { 32768, 12 };
static struct problem huge     =  { 32768, 24 };

/* Benchmark parameters. */      
int verbose = 0;                  /* Be verbose?        */    
int nthreads = 1;                 /* Number of threads. */
static struct problem *p = &tiny; /* Problem.           */  

/*
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: nb [options]\n");
	printf("Brief: N-Body Benchmark Kernel\n");
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
 * NB kernel.
 */
int main(int argc, char **argv)
{
	int i;               /* Loop index. */
	double max_force;    /* Maximum force */
	double sim_time;     /* Simulated time step */
	body_t* bodies;      /* Bodies */
	uint64_t end;        /* End time.   */
	uint64_t start;      /* Start time. */
	
#ifdef _XEON_PHI_
	double power;
#endif
	
	readargs(argc, argv);
	
	timer_init();
	omp_set_num_threads(nthreads);
	
	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");
	start = timer_get();
	prng_set_seed(123456789);

	/* Memory alloc */
	bodies = smalloc(p->nbodies*sizeof(body_t));

	sim_time = 0.0;
	/* Initialize with random numbers */
	for (i=0; i<p->nbodies; i++) {
		bodies[i].x	      = prngnum();
		bodies[i].y	      = prngnum();
		bodies[i].z	      = prngnum();
		bodies[i].mass    = 1.0;
		bodies[i].xold	  = bodies[i].x;
		bodies[i].yold	  = bodies[i].y;
		bodies[i].zold	  = bodies[i].z;
		bodies[i].fx	  = 0.0;
		bodies[i].fy	  = 0.0;
		bodies[i].fz	  = 0.0;
    }

	end = timer_get();
	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);
	
#ifdef _XEON_PHI_
	power_init();
#endif
	
	/* N-Body simulation */
	if (verbose)
		printf("simulating n-body movement...\n");

	start = timer_get();
	while (p->niter--) {
		max_force = compute_forces(bodies, p->nbodies);
		sim_time += compute_new_positions(bodies, p->nbodies, max_force);
	}
	end = timer_get();
	
#ifdef _XEON_PHI_
	power = power_end();
#endif

	if(verbose) {
		printf("Final positions\n");
		printf("   X    |    Y    |    Z   \n");
		for (i=0; i<p->nbodies; i++)
			printf("%.5lf | %.5lf | %.5lf\n", bodies[i].x, bodies[i].y, bodies[i].z);	
	}
	
	printf("timing statistics:\n");
	printf("  total time:    %f\n", timer_diff(start, end)*MICROSEC);

#ifdef _XEON_PHI_
	printf("  average power: %f\n", power*0.000001);
#endif
	
	free(bodies);
	return (EXIT_SUCCESS);
}
