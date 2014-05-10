/*
* Copyright(C) 2011-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
* 
* master.c - Master process.
*/

#include <assert.h>
#include <errno.h>
#include <globl.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector.h>
#include <string.h>

#include "timer.h"
#include "random.h"

/* Size of arrays. */
#define POINTS_SIZE (NUM_POINTS*DIMENSION)                  /* points[]      */
#define CENTROIDS_SIZE (NUM_CENTROIDS*DIMENSION)            /* centroids[]   */
#define POPULATION_SIZE (NUM_CENTROIDS)                     /* population[]  */
#define PCENTROIDS_SIZE (NUM_PROCS*NUM_CENTROIDS*DIMENSION) /* pcentroids[]  */
#define PPOPULATION_SIZE (NUM_PROCS*NUM_CENTROIDS)          /* ppopulation[] */

/* Delta of array sizes. */
#define DELTA (NUM_PROCS - 1)

/* K-means. */
static float points[POINTS_SIZE];                                   /* Data points.               */
static float centroids[CENTROIDS_SIZE + NUM_PROCS*DELTA*DIMENSION]; /* Data centroids.            */
static int population[POPULATION_SIZE + NUM_PROCS*DELTA];           /* Population of centroids.   */
static int map[NUM_POINTS];                                         /* Map of clusters.           */
static float pcentroids[PCENTROIDS_SIZE];                           /* Partial centroids.         */
static int ppopulation[PPOPULATION_SIZE];                           /* Partial population.        */
static int has_changed[NUM_PROCS*NUM_THREADS];                      /* Has any centroid changed?  */
static int too_far[NUM_PROCS*NUM_THREADS];                          /* Are points too far?        */
static int lnpoints[NUM_PROCS];                                     /* Local number of points.    */
static int lncentroids[NUM_PROCS];                                  /* Local number of centroids. */

/* Inter process communication. */
static int infd[NUM_PROCS];  /* Input channels.  */
static int outfd[NUM_PROCS]; /* Output channels. */


static RAND_STATE_T rand_state;

inline unsigned int simple_rng_rand()
{
	return simple_rng_next(&rand_state);
}

/*
* Initializes problem.
*/
static void init(void)
{
	int i; /* Loop index.    */
	int p; /* Working point. */

	rand_state = simple_rng_initialize(SEED);

	/* Initialize data points. */
	for (i = 0; i < POINTS_SIZE; i++)
		points[i] = simple_rng_rand()%DIMENSION;
	for (i = 0; i < NUM_POINTS; i++)
		map[i] = -1;

	/* Initialize centroids. */
	for (i = 0; i < NUM_CENTROIDS; i++)
	{
		vector_assign(CENTROID(i), POINT(p = simple_rng_rand()%NUM_POINTS));
		map[p] = i;
	}
	for (i = 0; i < NUM_POINTS; i++)
	{
		if (map[i] < 0)
			map[i] = simple_rng_rand()%NUM_CENTROIDS;
	}
}

/*
 * Sends work to slave processes.
 */
static void sendwork(void)
{
	int i;         /* Loop index.                   */
	ssize_t n;     /* Bytes to send/receive.        */
	ssize_t count; /* Bytes actually sent/received. */

	/* Distribute work among slave processes. */
	for (i = 0; i < NUM_PROCS; i++)
	{
		lnpoints[i] = ((i + 1) < NUM_PROCS) ? 
			NUM_POINTS/NUM_PROCS :  NUM_POINTS - i*(NUM_POINTS/NUM_PROCS);
			
		lncentroids[i] = ((i + 1) < NUM_PROCS) ? 
			NUM_CENTROIDS/NUM_PROCS : NUM_CENTROIDS - i*(NUM_CENTROIDS/NUM_PROCS);
		
	}

	/* Send work to slave processes. */
	for (i = 0; i < NUM_PROCS; i++)
	{
		n = sizeof(int);
		count = mppa_write(outfd[i], &lnpoints[i], n);
		assert(count != -1);
		
		n = lnpoints[i]*DIMENSION*sizeof(float);
		count = mppa_write(outfd[i], &points[i*(NUM_POINTS/NUM_PROCS)*DIMENSION], n);
		assert(count != -1);
		
		n = NUM_CENTROIDS*DIMENSION*sizeof(float);
		count = mppa_write(outfd[i], centroids, n);
		assert(count != -1);
		
		n = lnpoints[i]*sizeof(int);
		count = mppa_write(outfd[i], &map[i*(NUM_POINTS/NUM_PROCS)], n);
		assert(count != -1);
		
		n = NUM_PROCS*sizeof(int);
		count = mppa_write(outfd[i], lncentroids, n);
		assert(count != -1);
	}
}

#define PCENTROID(i, j) \
(&pcentroids[((i)*NUM_CENTROIDS + (j))*DIMENSION])


#define PPOPULATION(i, j) \
(&ppopulation[(i)*NUM_CENTROIDS + (j)])


/*
 * Synchronizes partial centroids.
 */
static void sync_pcentroids(void)
{
	int i, j;      /* Loop indexes.                 */
	ssize_t n;     /* Bytes to send/receive.        */
	ssize_t count; /* Bytes actually sent/received. */

	/* Receive partial centroids. */
	n = NUM_CENTROIDS*DIMENSION*sizeof(float);
	for (i = 0; i < NUM_PROCS; i++)
	{
		count = mppa_read(infd[i], PCENTROID(i, 0), n);
		assert(n == count);
	}

	/* 
	 * Send partial centroids to the
	 * slave process that is assigned to it.
	 */
	for (i = 0; i < NUM_PROCS; i++)
	{
		/* Build partial centroid. */
		n = lncentroids[i]*DIMENSION*sizeof(float);
		for (j = 0; j < NUM_PROCS; j++)
			memcpy(CENTROID(j*lncentroids[i]), PCENTROID(j, i*(NUM_CENTROIDS/NUM_PROCS)), n);

		n = NUM_PROCS*lncentroids[i]*DIMENSION*sizeof(float);
		count = mppa_write(outfd[i], centroids, n);
		assert(n == count);
	}
}

/*
 * Synchronizes partial population.
 */
static void sync_ppopulation(void)
{
	int i, j;      /* Loop indexes.                 */
	ssize_t n;     /* Bytes to send/receive.        */
	ssize_t count; /* Bytes actually sent/received. */

	/* Receive temporary population. */
	n = NUM_CENTROIDS*sizeof(int);
	for (i = 0; i < NUM_PROCS; i++)
	{
		count = mppa_read(infd[i], PPOPULATION(i, 0), n);
		assert(n == count);
	}

	/* 
	 * Send partial population to the
	 * slave process that assigned to it.
	 */
	for (i = 0; i < NUM_PROCS; i++)
	{
		/* Build partial population. */
		n = lncentroids[i]*sizeof(int);
		for (j = 0; j < NUM_PROCS; j++)
			memcpy(&population[j*lncentroids[i]], PPOPULATION(j, i*(NUM_CENTROIDS/NUM_PROCS)), n);

		n = NUM_PROCS*lncentroids[i]*sizeof(int);
		count = mppa_write(outfd[i], population, n);
		assert(n == count);
	}
}

/*
* Synchronizes centroids.
*/
static void sync_centroids(void)
{
	int i;         /* Loop index.                   */
	ssize_t n;     /* Bytes to send/receive.        */
	ssize_t count; /* Bytes actually sent/received. */

	/* Receive centroids. */
	for (i = 0; i < NUM_PROCS; i++)
	{
		n = lncentroids[i]*DIMENSION*sizeof(float);
		count = mppa_read(infd[i], CENTROID(i*(NUM_CENTROIDS/NUM_PROCS)), n);
		assert(n == count);
	}

	/* Broadcast centroids. */
	n = NUM_CENTROIDS*DIMENSION*sizeof(float);
	for (i = 0; i < NUM_PROCS; i++)
	{
		count = mppa_write(outfd[i], centroids, n);
		assert(n == count);
	}
}

/*
* Synchronizes slaves' status.
*/
static void sync_status(void)
{
	int i;         /* Loop index.                   */
	ssize_t n;     /* Bytes to send/receive.        */
	ssize_t count; /* Bytes actually sent/received. */

	/* Receive data. */
	n = NUM_THREADS*sizeof(int);
	for (i = 0; i < NUM_PROCS; i++)
	{
		count = mppa_read(infd[i], &has_changed[i*NUM_THREADS], n);
		assert(n == count);
		count = mppa_read(infd[i], &too_far[i*NUM_THREADS], n);
		assert(n == count);
	}

	/* Broadcast data to slaves. */
	n = NUM_PROCS*NUM_THREADS*sizeof(int);
	for (i = 0; i < NUM_PROCS; i++)
	{
		count = mppa_write(outfd[i], has_changed, n);
		assert(n == count);
		count = mppa_write(outfd[i], too_far, n);
		assert(n == count);
	}
}

/*
 * Asserts if another iteration is needed.
 */
static int again(void)
{
	int i;
	
	for (i = 0; i < NUM_PROCS*NUM_THREADS; i++)
	{
		if (has_changed[i] && too_far[i])
			return (1);
	}
	
	return (0);
}

/*
* Synchronizes kmeans.
*/
static int sync_kmeans(void)
{
	int it;

	it = 0;

	do
	{
		it++;

		sync_pcentroids();
		sync_ppopulation();
		sync_centroids();
		sync_status();

	} while (again());	

	return (it); 
}

/*
* Clusters data.
*/
int main(int argc, char **argv)
{
	int i;                    /* Loop index.           */
	int iterations;           /* Number of iterations. */
	pid_t pids[NUM_PROCS]; /* Processes IDs.        */
	char arg0[4];             /* Argument 0.           */
	char *args[2];            /* Arguments.            */
	char path[35];            /* Connector path.       */
	int sync_fd;              /* Sync file descriptor. */
	uint64_t match;           /* Match value for sync. */
	uint64_t start_time;      /* Start execution time. */
	uint64_t exec_time;       /* Execution time.       */

	((void)argc);
	((void)argv);

	/* Get initial time */
	start_time = mppa_get_time();

	init();

	match = -(1 << NUM_PROCS);
	sync_fd = mppa_open("/mppa/sync/128:64", O_RDONLY);
	assert(sync_fd != -1);
	assert(mppa_ioctl(sync_fd, MPPA_RX_SET_MATCH, match) != -1);

	/* Open channels. */
	for (i = 0; i < NUM_PROCS; i++)
	{		
		sprintf(path, "/mppa/channel/%d:%d/128:%d", i, i + 1, i + 1);
		outfd[i] = mppa_open(path, O_WRONLY);
		assert(outfd[i] != -1);
		sprintf(path, "/mppa/channel/128:%d/%d:%d", i + 33, i, i + 33);
		infd[i] = mppa_open(path, O_RDONLY);
		assert(infd[i] != -1);	
	}

	/* Spawn slaves. */
	args[1] = NULL;
	for (i = 0; i < NUM_PROCS; i++)
	{	
		sprintf(arg0, "%d", i);
		args[0] = arg0;
		pids[i] = mppa_spawn(i, NULL, "slave", (const char **)args, NULL);
		assert(pids[i] != -1);
	}

	/* Wait for slaves to be ready. */
	match = 0;
	assert(mppa_read(sync_fd, &match, 8) == 8);
	mppa_close(sync_fd);

	sendwork();

	iterations = sync_kmeans();

	/* Get final time */
	exec_time = mppa_diff_time(start_time, mppa_get_time());
	printf("%d;%d;%d;%f;%d;%d;%d;%f\n", NUM_POINTS, DIMENSION, NUM_CENTROIDS, MINDISTANCE, SEED, NUM_PROCS, iterations, exec_time/1000000.0);

	/* Close channels. */
	for (i = 0; i < NUM_PROCS; i++)
	{
		mppa_close(outfd[i]);
		mppa_close(infd[i]);
	}

	/* Join slaves. */
	for (i = 0; i < NUM_PROCS; i++)
		mppa_waitpid(pids[i], NULL, 0);

	return (0);
}
