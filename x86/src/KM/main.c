/* Kernel Include */
#include <timer.h>
#include <util.h>
#include "km.h"

/* C And MPPA Library Includes*/
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Problem. */
struct problem {
	int npoints;       /* Number of points.    */
	int ncentroids;    /* Number of centroids. */
};

/* Problem sizes. */
static struct problem tiny     = {  4096, 256};
static struct problem small    = {  8192, 512};
static struct problem standard = { 16384, 1024};
static struct problem large    = { 32768, 1024};
static struct problem huge     = { 65536, 1024};

static int seed = 0;              /* Seed number. */      
static struct problem *p = &tiny; /* Problem. */   
int verbose = 0;                  /* Be verbose? */            
int nthreads = 1;                 /* Number of threads. */       

/* Prints program usage and exits. */
static void usage() {
	printf("Usage: kmeans [options]\n");
	printf("Brief: Kmeans Benchmark Kernel\n");
	printf("Options:\n");
	printf("  --help             Display this information and exit\n");
	printf("  --nthreads <value> Set number of threads\n");
	printf("  --class <name>     Set problem class:\n");
	printf("                       - small\n");
	printf("                       - standard\n");
	printf("                       - large\n");
	printf("                       - huge\n");
	printf("  --verbose          Be verbose\n");
	exit(0);
}

/* Reads command line arguments. */
static void readargs(int argc, char **argv) {
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

/* Runs benchmark. */
int main(int argc, char **argv) {
	int i;               /* Loop index.             */
	int *map;            /* Map of clusters.        */
	int dimension;       /* Problem dimension.      */
	float *points;       /* Points being clustered. */
	uint64_t start, end; /* Start and End time.     */

#ifdef _XEON_PHI_
	double power;
#endif

	readargs(argc, argv);

	timer_init();
	srandnum(seed);
	omp_set_num_threads(nthreads);

	/* Setting the dimension for the problem. */
	dimension = 16; 

	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");
	start = timer_get();
	points = smalloc(p->npoints*dimension*sizeof(float));
	for (i = 0; i < p->npoints * dimension; i++)
		points[i] = randnum() & 0xffff;
	end = timer_get();

	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);

#ifdef _XEON_PHI_
	power_init();
#endif

	/* Cluster data. */
	if (verbose)
		printf("clustering data...\n");
	start = timer_get();
	map = kmeans(points, p->npoints, p->ncentroids, dimension);
	end = timer_get();

#ifdef _XEON_PHI_
	power = power_end();
#endif

	printf("timing statistics:\n");
	printf("  total time:    %f\n", timer_diff(start, end)*MICROSEC);

#ifdef _XEON_PHI_
	printf("  average power: %f\n", power*0.000001);
#endif
	
	/* House keeping. */
	free(map);
	free(points);
	
	return (0);

}