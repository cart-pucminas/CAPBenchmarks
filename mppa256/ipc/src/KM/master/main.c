#include <arch.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <util.h>
#include <timer.h>

/* Clusters data. */
extern int *kmeans(float *_points, int _npoints, int _ncentroids, int _dimension);

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

/* Benchmark parameters. */
int verbose = 0;                  /* Be verbose?        */
static int seed = 0;              /* Seed value.        */
int nclusters = 1;                 /* Number of threads. */
static struct problem *p = &tiny; /* Problem.           */

/*
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: kmeans [options]\n");
	printf("Brief: Kmeans Benchmark Kernel\n");
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

/* Runs benchmark. */
int main(int argc, char **argv) {
	int i;               /* Loop index.      */
	int *map;            /* Map of clusters. */
	int dimension;       /* Dimension of points. */
	float *points;       /* Data points.     */
	uint64_t start, end; /* End time.        */

	readargs(argc, argv);

	timer_init();
	srandnum(seed);

	/* Setting the dimension for the problem. */
	dimension = 16; 

	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");
	start = timer_get();
	points = smalloc(p->npoints * dimension * sizeof(float));
	for (i = 0; i < p->npoints * dimension; i++)
		points[i] = randnum() & 0xffff;
	end = timer_get();

	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);

	/* Cluster data. */
	if (verbose)
		printf("clustering data...\n");
	start = timer_get();
	map = kmeans(points, p->npoints, p->ncentroids, dimension);
	end = timer_get();
	total = timer_diff(start, end);

	/* Print timing statistics. */
	printf("timing statistics:\n");
	printf("  master:        %f\n", master*MICROSEC);
	for (i = 0; i < nclusters; i++)
		printf("  slave %d:      %f\n", i, slave[i]*MICROSEC);
	printf("  spawn %d CC:    %f\n", nclusters, spawn*MICROSEC);
	printf("  communication: %f\n", communication*MICROSEC);
	printf("  total time:    %f\n", total*MICROSEC);
	printf("data exchange statistics:\n");
	printf("  data sent:            %d\n", data_sent);
	printf("  number sends:         %u\n", nsend);
	printf("  data received:        %d\n", data_received);
	printf("  number receives:      %u\n", nreceive);

	/* House keeping. */
	free(map);
	free(points);

	return (0);
}
