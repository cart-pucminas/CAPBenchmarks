/*
* Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
* 
* master.c - Master process.
*/

#include <assert.h>
#include <mppaipc.h>
#include <mppa/osconfig.h>
#include <stdio.h>
#include <stdint.h>
#include "integer-sort.h"

/*
 * Process table.
 */
static struct process
{
	pid_t pid;    /* Process ID.     */
	int outfd;    /* Output channel. */
	int infd;     /* Input channel.  */
	void *chunkp; /* Chunk pointer.  */
} proctab[NUM_PROCS];

/*
 * Global sync file descriptor.
 */
int sync_fd; 

/*============================================================================*
 *                             Utility Functions                              *
 *============================================================================*/

/*
 * Prints an error message and exits.
 */
static void error(const char *msg)
{
	fprintf(stderr, "error: %s\n", msg);
	exit(-1);
}

/*
 * Safely allocates memory.
 */
void *smalloc(size_t size)
{
	void *ptr;

	if ((ptr = malloc(size)) == NULL)
		error("cannot malloc()");

	return (ptr);
}

/*
 * Generates an array of random integers.
 */
void array_random(int *array, int n)
{
	int i;

	/* Generate array. */
	for (i = 0; i < n; i++)
		array[i] = rand();
}

#define MPPA_FREQUENCY 400

static uint64_t residual_error = 0;


#define mppa_get_time() \
	(__k1_io_read64((void *)0x70084040) / MPPA_FREQUENCY)

#define mppa_diff_time(t1, t2) \
	(t2 - t1 - residual_error)

void mppa_init_time(void) {
  uint64_t t1, t2;
  t1 = mppa_get_time();
  t2 = mppa_get_time();
  residual_error = t2 - t1;
}



/*============================================================================*
 *                               Integer Sort                                 *
 *============================================================================*/

/*
 * Sends number to slave processes.
 */
static void sendchunk(int id, int *array, int i)
{
	ssize_t count;     /* Bytes actually sent. */
	struct process *p; /* Process.             */
	
	/* Set write parameters. */
	p = &proctab[id];
	p->chunkp = &array[i*CHUNK_SIZE];
	
	count = mppa_write(p->outfd, p->chunkp, CHUNK_SIZE*sizeof(int));
	assert(count != -1);
}

/*
 * Sorts an array of integers.
 */
static void sort(int *array, int n)
{
	int i;       /* Loop index.       */
	int nsent;   /* Number of sent*/
	int nchunks; /* Number of chunks. */
	
	nchunks = n/CHUNK_SIZE;
	
	/* Sort */
	nsent = 0;
	for (i = 0; i < nchunks; i++)
	{
		sendchunk(nsent++, array, i);
		
		/* Wait for chunks. */
		if (nsent >= NUM_PROCS)
		{
			nsent = 0;
		}
	}
}


/*
 * Spwans slave processes.
 */
static void spawn_slaves(void)
{
	int i;          /* Loop index.     */
	char arg0[4];   /* Argument 0.     */
	char *args[2];  /* Arguments.      */

	/* Spawn slaves. */
	args[1] = NULL;
	for (i = 0; i < NUM_PROCS; i++)
	{	
		sprintf(arg0, "%d", i);
		args[0] = arg0;
		proctab[i].pid = mppa_spawn(i, NULL, "slave", (const char **)args, NULL);
		assert(proctab[i].pid != -1);
	}
}

/*
 * Joins slave processes.
 */
static void join_slaves(void)
{
	int i;
	
	/* Join slaves. */
	for (i = 0; i < NUM_PROCS; i++)
		mppa_waitpid(proctab[i].pid, NULL, 0);
}

/*
 * Waits for slaves to be ready.
 */
static void sync_slaves(void)
{
	uint64_t match;

	assert(mppa_read(sync_fd, &match, 8) == 8);
}

/*
 * Open NoC connectors.
 */
static void open_noc_connectors(void)
{
	int i;          /* Loop index.     */
	char path[35];  /* Connector path. */
	uint64_t match; /* match value.    */
	
	match = -(1 << NUM_PROCS);
	sync_fd = mppa_open("/mppa/sync/128:64", O_RDONLY);
	assert(sync_fd != -1);
	assert(mppa_ioctl(sync_fd, MPPA_RX_SET_MATCH, match) != -1);

	/* Open channels. */
	for (i = 0; i < NUM_PROCS; i++)
	{		
		sprintf(path, "/mppa/channel/%d:%d/128:%d", i, i + 1, i + 1);
		proctab[i].outfd = mppa_open(path, O_WRONLY);
		assert(proctab[i].outfd != -1);
	}
}

/*
 * Close NoC connectors.
 */
static void close_noc_connectors(void)
{
	int i;
	
	/* Close channels. */
	for (i = 0; i < NUM_PROCS; i++)
		mppa_close(proctab[i].outfd);
	
	/* Close sync. */
	mppa_close(sync_fd);
}


int main(int argc, char **argv)
{
	int *array;     /* Array.      */
	uint64_t start; /* Start time. */
	uint64_t end;   /* End time.   */
	
	((void)argc);
	((void)argv);
	
	mppa_init_time();

	array = smalloc(PROBLEM_SIZE*sizeof(int));
	
	fprintf(stderr, "Initializing...\n");
	
	start = mppa_get_time();
	array_random(array, PROBLEM_SIZE);
	end = mppa_get_time();
	
	fprintf(stderr, "Time to initialize: %lf\n", mppa_diff_time(start, end)/1000000.0);

	open_noc_connectors();	
	spawn_slaves();
	sync_slaves();

	fprintf(stderr, "Sorting...\n");

	start = mppa_get_time();
	
	sort(array, PROBLEM_SIZE);

	close_noc_connectors();
	join_slaves();
	
	end = mppa_get_time();
	
	fprintf(stderr, "Time to sort: %lf\n", mppa_diff_time(start, end)/1000000.0);
	
	/* House keeping. */
	free(array);

	return (0);
}
