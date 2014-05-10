/*
 * Copyright(C) 2013-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * main.c - main() implementation.
 */

#include <algorithm.h>
#include <array.h>
#include <globl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>
#include <vector.h>
#include <stdint.h>

#include "timer.h"
#include "random.h"

/* Program parameters. */
unsigned seed = 0;       /* Seed value.            */
int verbose = 0;         /* Be verbose?            */
int npoints = 0;         /* Number of data points. */
int dimension = 0;       /* Dimension of points.   */
int ncentroids = 0;      /* Number of centroids.   */
float mindistance = 0.0; /* Minimum distance.      */

/* static unsigned next = 1; */
static RAND_STATE_T rand_state;

inline unsigned int simple_rng_rand()
{
  return simple_rng_next(&rand_state);
}

/*
 * Prints program usage and exit.
 */
static void usage(char *execname)
{
	printf("Usage: %s [options]\n", execname);
	printf("Options:\n");
	printf("  --dimension <value>     Data point dimension\n");
	printf("  --help                  Display this information\n");
	printf("  --num-centroids <value> Set the number of data centroids\n");
	printf("  --min-distance <value>  Set the minimum distance\n");
	printf("  --num-points <value>    Set the number of data points\n");
	printf("  --num-threads <value>   Set the number of working threads\n");
	printf("  --seed <value>          Seed for random choices\n");
	printf("  --verbose               Be verbose\n");
	
	exit(EXIT_SUCCESS);
}

/*
 * Reads command line arguments.
 */
static void getargs(int argc, char **argv)
{
	int i;     /* Loop index.       */
	char *arg; /* Current argument. */
	int state; /* Processing state. */
	
	/* State values. */
	#define READ_ARG        0
	#define SET_NCENTROIDS  1
	#define SET_MINDISTANCE 2
	#define SET_SEED        3
	#define SET_DIMENSION   4
	#define SET_NTHREADS    5
	#define SET_NPOINTS     6
	
	state = READ_ARG;
	
	/* Read all command line arguments. */
	for (i = 1; i < argc; i++)
	{
		arg = argv[i];
		
		/* Set value. */
		if (state != READ_ARG)
		{
			switch (state)
			{
				/* Set number of clusters. */
				case SET_NCENTROIDS :
					ncentroids = atoi(arg);
					state = READ_ARG;
					break;
				
				/* Set minimum distance. */
				case SET_MINDISTANCE :
					mindistance = atof(arg);
					state = READ_ARG;
					break;
				
				/* Seed seed value. */
				case SET_SEED :
					seed = atoi(arg);
					state = READ_ARG;
					break;
				
				/* Set window width. */
				case SET_DIMENSION :
					dimension = atoi(arg);
					state = READ_ARG;
					break;
				
				/* Set number of threads. */
				case SET_NTHREADS :
					nthreads = atoi(arg);
					state = READ_ARG;
					break;
					
				/* Set number of points. */
				case SET_NPOINTS :
					npoints = atoi(arg);
					state = READ_ARG;
					break;
				
				default:
					usage(argv[0]);
			}
			
			continue;
		}
		
		/* Parse command line argument. */
		if (!strcmp(arg, "--num-centroids"))
			state = SET_NCENTROIDS;
		else if (!strcmp(arg, "--min-distance"))
			state = SET_MINDISTANCE;
		else if (!strcmp(arg, "--verbose"))
			verbose = 1;
		else if (!strcmp(arg, "--seed"))
			state = SET_SEED;
		else if (!strcmp(arg, "--dimension"))
			state = SET_DIMENSION;
		else if (!strcmp(arg, "--num-threads"))
			state = SET_NTHREADS;
		else if (!strcmp(arg, "--num-points"))
			state = SET_NPOINTS;
		else
			usage(argv[0]);
	}
	
	/* Invalid command line arguments. */
	if (ncentroids < 1)
		usage(argv[0]);
	else if (mindistance < 0)
		usage(argv[0]);
	else if (dimension < 1)
		usage(argv[0]);
	else if (nthreads < 1)
		usage(argv[0]);
}

/*
 * Reads input and launches application.
 */
int main(int argc, char **argv)
{
	int i;        /* Loop index.     */
	int *map;     /* map of cluster. */
	array_t data; /* Data points.    */
	uint64_t start, end, exec_time;

	getargs(argc, argv);
	
	rand_state = simple_rng_initialize(seed);

	printf("%d;%d;%d;%f;%d;%d;", npoints, dimension, ncentroids, mindistance, seed, nthreads);
	
	start = get_time();

	data = array_create(&vector, npoints);
	
	/* Failed to create array. */
	if (data == NULL)
		error("cannot create data points");
	
	if (verbose)
	{
	    printf("Number of data points: %d\n", npoints);
	    printf("Data dimension:        %d\n", dimension);
	    printf("Number of clusters:    %d\n", ncentroids);
	    printf("Minimum distance:      %f\n", mindistance);
	    printf("Seed value:            %u\n\n", seed);
     }

	/* Generate random data points. */
	for (i = 0; i < npoints; i++)
	{
		ARRAY(data, i) = vector_random(dimension);
		
		/* Failed to create point. */
		if (ARRAY(data, i) == NULL)
			error("failed to create data point");
	}
	
	map = kmeans(data, ncentroids, mindistance);
	
	/* Failed to run kmeans(). */
	if (map == NULL)
		error("cannot cluster data points");

	/* House keeping. */
	free(map);
	array_destroy(data);

	end = get_time();
	exec_time = diff_time(start, end);

	printf ("%f", exec_time/1000000.0);

	return (EXIT_SUCCESS);
}
