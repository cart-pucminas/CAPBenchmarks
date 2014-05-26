/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * slave.c - k-means slave process.
 */

#include <assert.h>
#include <globl.h>
#include <mppa/osconfig.h>
#include <stdio.h>
#include <string.h>
#include <vector.h>
#include <omp.h>

/* Size of arrays. */
#define MAP_SIZE (NUM_POINTS/NUM_PROCS)                       /* map[]         */
#define POINTS_SIZE ((NUM_POINTS/NUM_PROCS)*DIMENSION)        /* points[]      */
#define CENTROIDS_SIZE (NUM_CENTROIDS*DIMENSION)              /* centroids[]   */
#define PPOPULATION_SIZE (NUM_CENTROIDS)                      /* ppopulation[] */
#define LCENTROIDS_SIZE ((NUM_CENTROIDS/NUM_PROCS)*DIMENSION) /* lcentroids[]  */

/* Delta of array sizes. */
#define DELTA (NUM_PROCS - 1)

/* K-means. */

static float points[POINTS_SIZE + DELTA*DIMENSION];                 /* Data points.               */
static float centroids[CENTROIDS_SIZE + NUM_PROCS*DELTA*DIMENSION]; /* Data centroids.            */
static int map[MAP_SIZE + DELTA];                                   /* Map of clusters.           */
static int too_far[NUM_PROCS*NUM_THREADS];                          /* Are points too far?        */
static int has_changed[NUM_PROCS*NUM_THREADS];                      /* Hash any centroid changed? */
static int lncentroids[NUM_PROCS];                                  /* Local number of centroids. */
static int lnpoints;                                                /* Local number of points.    */
static int ppopulation[PPOPULATION_SIZE + NUM_PROCS*DELTA];         /* Partial population.        */
static float lcentroids[LCENTROIDS_SIZE + DELTA*DIMENSION];         /* Local centroids.           */

/* Inter process communication. */
static int rank;  /* Process rank.   */
static int infd;  /* Input channel.  */
static int outfd; /* Output channel. */

/* Thread communication. */
static omp_lock_t lock[NUM_THREADS];

/*============================================================================*
 *                                populate()                                 *
 *============================================================================*/

/*
 * Populates clusters.
 */
static void populate(void)
{
	int i, j;        /* Loop indexes.       */
	float tmp;      /* Auxiliary variable. */
	float distance; /* Smallest distance.  */
	
	memset(&too_far[rank*NUM_THREADS], 0, NUM_THREADS*sizeof(int));
	
	/* Iterate over data points. */
	#pragma omp parallel for schedule(static) default(shared) private(i, j, tmp, distance)
	for (i = 0; i < lnpoints; i++)
	{
		distance = vector_distance(CENTROID(map[i]), POINT(i));
		
		/* Look for closest cluster. */
		for (j = 0; j < NUM_CENTROIDS; j++)
		{
			/* Point is in this cluster. */
			if (j == map[i])
				continue;
				
			tmp = vector_distance(CENTROID(j), POINT(i));
			
			/* Found. */
			if (tmp < distance)
			{
				map[i] = j;
				distance = tmp;
			}
		}
		
		/* Cluster is too far away. */
		if (distance > MINDISTANCE)
			too_far[rank*NUM_THREADS + omp_get_thread_num()] = 1;
	}	
}

/*============================================================================*
 *                          compute_centroids()                               *
 *============================================================================*/

/*
 * Returns the population for clusther ith, vector jth.
 */
#define POPULATION(i, j) \
	(&ppopulation[(i)*lncentroids[rank] + (j)])

/*
 * Returns the jth input centroid from the ith cluster.
 */
#define PCENTROID(i, j) \
	(&centroids[((i)*lncentroids[rank] + (j))*DIMENSION])

/*
 * Returns the ith local centroid.
 */
#define LCENTROID(i) \
	(&lcentroids[(i)*DIMENSION])

/*
 * Synchronizes partial centroids.
 */
static void sync_pcentroids(void)
{	
	ssize_t n;     /* Bytes to send/receive.        */
	ssize_t count; /* Bytes actually sent/received. */
	
	/* Send partial centroids. */
	n = NUM_CENTROIDS*DIMENSION*sizeof(float);
	count = mppa_write(outfd, centroids, n);
	assert(n == count);
	
	/* Receive partial centroids. */
	n = NUM_PROCS*lncentroids[rank]*DIMENSION*sizeof(float);
	count = mppa_read(infd, centroids, n);
	assert(n == count);
}

/*
 * Synchronizes partial population.
 */
static void sync_ppopulation(void)
{
	ssize_t n;     /* Bytes to send/receive.        */
	ssize_t count; /* Bytes actually sent/received. */
		
	/* Send partial population. */
	n = NUM_CENTROIDS*sizeof(int);
	count = mppa_write(outfd, ppopulation, n);
	assert(n == count);
	
	/* Receive partial population. */
	n = NUM_PROCS*lncentroids[rank]*sizeof(int);
	count = mppa_read(infd, ppopulation, n);
	assert(n == count);
}

/*
 * Synchronizes centroids.
 */
static void sync_centroids(void)
{
	ssize_t n;     /* Bytes to send/receive.        */
	ssize_t count; /* Bytes actually sent/received. */
	
	n = lncentroids[rank]*DIMENSION*sizeof(float);
	count = mppa_write(outfd, lcentroids, n);
	assert(n == count);
	
	n = NUM_CENTROIDS*DIMENSION*sizeof(float);
	count = mppa_read(infd, centroids, n);
	assert(n == count);
}

/*
 * Synchronizes status.
 */
static void sync_status(void)
{
	ssize_t n;
	ssize_t count;
	
	n = NUM_THREADS*sizeof(int);
	count = mppa_write(outfd, &has_changed[rank*NUM_THREADS], n);
	assert(n == count);
	count = mppa_write(outfd, &too_far[rank*NUM_THREADS], n);
	assert(n == count);
		
	n = NUM_PROCS*NUM_THREADS*sizeof(int);
	count = mppa_read(infd, has_changed, n);
	assert(n == count);
	count = mppa_read(infd, too_far, n);
	assert(n == count);
}


/*
 * Computes clusters' centroids.
 */
static void compute_centroids(void)
{
	int i, j;       /* Loop indexes.        */
	int population; /* Centroid population. */

	memcpy(lcentroids, CENTROID(rank*(NUM_CENTROIDS/NUM_PROCS)), lncentroids[rank]*DIMENSION*sizeof(float));
	memset(&has_changed[rank*NUM_THREADS], 0, NUM_THREADS*sizeof(int));
	memset(centroids, 0, (NUM_CENTROIDS + DELTA*NUM_PROCS)*DIMENSION*sizeof(float));
	memset(ppopulation, 0, (NUM_CENTROIDS + NUM_PROCS*DELTA)*sizeof(int));

	/* Compute partial centroids. */
	#pragma omp parallel for schedule(static) default(shared) private(i, j)
	for (i = 0; i < lnpoints; i++)
	{
		j = map[i]%NUM_THREADS;
		
		omp_set_lock(&lock[j]);
		
		vector_add(CENTROID(map[i]), POINT(i));
			
		ppopulation[map[i]]++;
		
		omp_unset_lock(&lock[j]);
	}
		
	sync_pcentroids();
	sync_ppopulation();
	
	/* Compute centroids. */
	#pragma omp parallel for schedule(static) default(shared) private(i, j, population)
	for (j = 0; j < lncentroids[rank]; j++)
	{
		population = 0;
		
		for (i = 0; i < NUM_PROCS; i++)
		{
			if (*POPULATION(i, j) == 0)
				continue;
			
			population += *POPULATION(i, j);
			
			if (i == rank)
				continue;
			
			vector_add(PCENTROID(rank, j), PCENTROID(i, j));
		}
		
		if (population > 1)
			vector_mult(PCENTROID(rank, j), 1.0/population);
		
		/* Cluster mean has changed. */
		if (!vector_equal(PCENTROID(rank, j), LCENTROID(j)))
		{
			has_changed[rank*NUM_THREADS + omp_get_thread_num()] = 1;
			vector_assign(LCENTROID(j), PCENTROID(rank, j));
		}
	}
	
	sync_centroids();
		
	sync_status();
}

/*============================================================================*
 *                                 again()                                    *
 *============================================================================*/

/*
 * Asserts if another iteration is needed.
 */
static int again(void)
{
	int i;
	
	/* Checks if another iteration is needed. */	
	for (i = 0; i < NUM_PROCS*NUM_THREADS; i++)
	{
		if (has_changed[i] && too_far[i])
			return (1);
	}
			
	return (0);
}

/*============================================================================*
 *                                kmeans()                                    *
 *============================================================================*/

/*
 * Clusters data.
 */
static void kmeans(void)
{	
	int i;
	int it = 0;

	omp_set_num_threads(NUM_THREADS);
	for (i = 0; i < NUM_THREADS; i++)
		omp_init_lock(&lock[i]);
	
	/* Cluster data. */
	do
	{	
		it++;
		populate();
		compute_centroids();
	} while (again());
}

/*============================================================================*
 *                                 getwork()                                  *
 *============================================================================*/

/*
 * Receives work from master process.
 */
static void getwork(void)
{	
	ssize_t n;     /* Bytes to send/receive.        */
	ssize_t count; /* Bytes actually sent/received. */
	
	n = sizeof(int);
	count = mppa_read(infd, &lnpoints, n);
	assert(n == count);
	
	n = lnpoints*DIMENSION*sizeof(float);
	count = mppa_read(infd, points, n);
	assert(n == count);
	
	n = NUM_CENTROIDS*DIMENSION*sizeof(float);
	count = mppa_read(infd, centroids, n);
	assert(n == count);
	
	n = lnpoints*sizeof(int);
	count = mppa_read(infd, map, n);
	assert(n == count);
	
	n = NUM_PROCS*sizeof(int);
	count = mppa_read(infd, lncentroids, n);
	assert(count != -1);
}

/*============================================================================*
 *                                  main()                                    *
 *============================================================================*/

/*
 * Clusters data.
 */
int main(int argc, char **argv)
{
	char path[35];
	uint64_t mask; /* Mask for sync.        */
	int sync_fd;   /* Sync file descriptor. */
	
	((void)argc);
	
	rank = atoi(argv[0]);
	
	sync_fd = mppa_open("/mppa/sync/128:64", O_WRONLY);
	assert(sync_fd != -1);
	
	/* Open channels. */
	sprintf(path, "/mppa/channel/%d:%d/128:%d", rank, rank + 1, rank + 1);
	infd = mppa_open(path, O_RDONLY);
	assert(infd != -1);
	sprintf(path, "/mppa/channel/128:%d/%d:%d", rank + 33, rank, rank + 33);
	outfd = mppa_open(path, O_WRONLY);
	assert(outfd != -1);
	
	/* Synchronize with master. */
	mask = 1 << rank;
	assert(mppa_write(sync_fd, &mask, 8) == 8);
	
	getwork();
	
	kmeans();
	
	/* Close channels. */
	mppa_close(sync_fd);
	mppa_close(infd);
	mppa_close(outfd);
	
	mppa_exit(0);
	return (0);
}
