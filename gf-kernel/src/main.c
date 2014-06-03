/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * Gaussian Filter Benchmark Kernel.
 */

#include <global.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <util.h>

#ifndef _MPPA_256_
#include <omp.h>
#endif

/*
 * Gaussian filter.
 */
extern void gauss_filter
(unsigned char *img, int imgsize, double *mask, int masksize);

/*
 * Problem.
 */
struct problem
{
	int masksize; /* mask size.  */
	int imgsize;  /* Image size. */
};

/* Problem sizes. */
static struct problem tiny        = {  5,  1024 };
static struct problem small       = {  7,  2048 };
static struct problem workstation = {  9,  4096 };
static struct problem standard    = {  9, 16384 };
static struct problem large       = { 13, 32768 };

/* Benchmark parameters. */
int verbose = 0;                  /* Be verbose?        */
int nthreads = 1;                 /* Number of threads. */
static int seed = 0;              /* Seed number.       */
static struct problem *p = &tiny; /* Problem.           */

/*
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: gf [options]\n");
	printf("Brief: Gaussian Filter Benchmark Kernel\n");
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
 * Generates mask.
 */
static void generate_mask(double *mask)
{
	int half;
	int i, j;
	double sec;
	double first;
	double total;
	
	first = 1.0/(2.0*PI*SD*SD);
	half = p->masksize >> 1;
	total = 0;
	
	#define MASK(i, j) \
		mask[(i)*p->masksize + (j)]

	for (i = -half; i <= half; i++)
	{
		for (j = -half; j <= half; j++)
		{
			sec = -((i*i + j*j)/2.0*SD*SD);
			sec = pow(E, sec);

			MASK(i + half, j + half) = first*sec;
			total += MASK(i + half, j + half);
		}
	}
	
	for (i = 0 ; i < p->masksize; i++)
	{
		for (j = 0; j < p->masksize; j++)
			MASK(i, j) /= total;
	}
}

/*
 * Runs benchmark.
 */
int main(int argc, char **argv)
{
	int i;              /* Loop index. */
	double *mask;       /* Mask.       */
	uint64_t end;       /* End time.   */
	uint64_t start;     /* Start time. */
	unsigned char *img; /* Image.      */
	
	readargs(argc, argv);
	
	timer_init();
	srandnum(seed);
#ifndef _MPPA_256_	
	omp_set_num_threads(nthreads);
#endif
	
	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");
	start = timer_get();
	img = smalloc(p->imgsize*p->imgsize*sizeof(char));
	for (i = 0; i < p->imgsize*p->imgsize; i++)
		img[i] = randnum() & 0xff;
	mask = smalloc(p->masksize*p->masksize*sizeof(double));
	generate_mask(mask);
	end = timer_get();
	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);
		
	/* Apply filter. */
	if (verbose)
		printf("applying filter...\n");
	start = timer_get();
	gauss_filter(img, p->imgsize, mask, p->masksize);
	end = timer_get();
	if (verbose)
		printf("  time spent: ");
	printf("%f\n", timer_diff(start, end)*MICROSEC);
	
	/* House keeping. */
	free(mask);
	free(img);
	
	return (0);
}
