/* Kernel Includes */
#include <async_util.h>
#include <spawn_util.h>
#include <problem.h>
#include <global.h> 
#include <timer.h>
#include <util.h>
#include "master.h"

/* C And MPPA Library Includes*/
#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h> 
#include <mppa_async.h>
#include <unistd.h> 
#include <mppa_power.h>
#include <utask.h>
#include <omp.h>

/*
 * Wrapper to data_send(). 
 */
#define data_send(b, c)                   \
{                                        \
	data_sent += c;                      \
	nsend++;                             \
	communication += data_send(a, b, c); \
}                                        \

/*
 * Wrapper to data_receive(). 
 */
#define data_receive(b, c)                   \
{                                           \
	data_received += c;                     \
	nreceive++;                             \
	communication += data_receive(a, b, c); \
}     

typedef struct {
	int number;
	int num;
	int den;
} Item;

#define MAX_TASK_SIZE 65536

static Item tasks[MAX_TASK_SIZE];

/* Async Segments. */
static mppa_async_segment_t times_segment;
static mppa_async_segment_t tasks_segment;

/* Slave tasks offsets */
static off64_t offsets[NUM_CLUSTERS];

/* Parameters.*/
static int startnum;               /* Start number.      */
static int endnum;                 /* End number.        */
static int problemsize;            /* Total task size    */
static int avgtasksize;            /* Average task size. */
static int tasksize[NUM_CLUSTERS]; /* Task size.         */

static void distributeTaskSizes(int _start, int _end) {
	startnum = _start;
	endnum = _end;

	problemsize = (_end - _start + 1);

	if (problemsize > MAX_TASK_SIZE)
		problemsize = MAX_TASK_SIZE;

	avgtasksize = problemsize/nclusters;
	
	/* Distribute task sizes. */
	for (int i = 0; i < nclusters; i++)
		tasksize[i] = (i + 1 < nclusters)?avgtasksize:problemsize-i*avgtasksize;
}

static void setOffsets() {
	off64_t offset = 0;
	for (int i = 0; i < nclusters; i++) {
		offsets[i] = offset;
		offset += tasksize[i] * sizeof(Item);
	}
}

static void setTasks() {
	int aux = startnum;
	for (int i = 0; i < problemsize; i++)
		tasks[i].number = aux++; 
}

static void createSegments() {
	mppa_async_segment_create(&tasks_segment, 1, &tasks, problemsize * sizeof(Item), 0, 0, NULL);
	mppa_async_segment_create(&times_segment, 2, &slave, nclusters * sizeof(uint64_t), 0, 0, NULL);
}

static void sendWork() {
	int i;               /* Loop indexes. */
	omp_set_dynamic(0);
	#pragma omp parallel for private(i) default(shared) num_threads(3)
	for (i = 0; i < nclusters; i++) {
		char str_size[10], str_offset[50];
		sprintf(str_size, "%d", tasksize[i]);
		sprintf(str_offset, "%lld", offsets[i]);
		char *args[3];
		args[0] = str_size;
		args[1] = str_offset;
		args[2] = NULL; 
		
		/* Spawning PE0 of cluster i*/
		spawn_slave(i, args);
	}
	omp_set_dynamic(1);
}

static void waitCompletion() {
	for (int i= 0; i < nclusters; i++)
		mppa_async_evalcond((long long *)&slave[i], 0, MPPA_ASYNC_COND_GT, 0);
	mppa_async_fence(&tasks_segment, NULL);
}

static void joinAll() {
	for (int i = 0; i < nclusters; i++)
		join_slave(i);
}

/*
 * Thread's data.
 */
struct tdata
{
	/* Thread ID. */
	pthread_t tid; 
	 
	struct
	{
		int i0;
		int in;
	} args;
	
	struct
	{
		int nfriends;
	} result;
} tdata[NUM_IO_CORES];

/*
 * Thread's main.
 */
static void *thread_main(void *args)
{
	int i, j;        /* Loop indexes.      */
	int nfriends;    /* Number of friends. */
	struct tdata *t; /* Thread data.       */
	
	t = args;
	
	/* Count number of friends. */
	nfriends = 0;
	for (i = t->args.i0; i < t->args.in; i++)
	{
		for (j = 0; j < i; j++)
		{
			/* Friends. */
			if ((tasks[i].num == tasks[j].num) && 
			(tasks[i].den == tasks[j].den))
				nfriends++;
		}	
	}
	
	t->result.nfriends = nfriends;
	
	pthread_exit(NULL);
	return (NULL);
}

/*
 * Counts friendly numbers.
 */
static int count_friends(void)
{
	int i;               /* Loop index.       */
	int nfriends;        /* Number friends.   */
	uint64_t start, end; /* Timers.           */
	int chunksize;       /* Thread work size. */
	struct tdata *t;     /* Thread data.       */
	
	start = timer_get();
	
	/* Spwan slave threads. */
	chunksize = (endnum - startnum + 1)/NUM_IO_CORES;
	for (i = 0; i < NUM_IO_CORES; i++)
	{
		t[i].args.i0 = (i == 0) ? startnum + 1 : i*chunksize;
		t[i].args.in = (i + 1 < NUM_IO_CORES) ? (i + 1)*chunksize :
		(endnum - startnum + 1);
		pthread_create(&t[i].tid, NULL, thread_main, (void *)&t[i]);
	}
	
	/* Join threads. */
	for (i = 0; i < NUM_IO_CORES; i++)
		pthread_join(t[i].tid, NULL);	
	
	/* Reduce. */
	nfriends = 0;
	for (i = 0; i < NUM_IO_CORES; i++)
		nfriends += t[i].result.nfriends;

	end = timer_get();
	master += timer_diff(start, end);

	return (nfriends);
}

static void test() {
	int aux = 0;
	for (int i = 0; i < nclusters; i++) {
		for (int j = aux; j < aux + 3; j++)
			printf("Task[%d] = %d || %d || %d\n", i, tasks[j].number, tasks[j].num, tasks[j].den);
		aux += tasksize[i];
	}
}

void friendly_numbers(int _start, int _end) {
	/* Intervals to each cluster */
	distributeTaskSizes(_start, _end);

	/* Initializes async server */
	async_master_init();
	async_master_start();

	/* Initializes tasks number values */
	setTasks();

	/* Set clusters tasks offsets */
	setOffsets();

	/* Creating segments to get tasks returns values*/
	createSegments();

	/* Spawns clusters and send them work */
	sendWork();

	/* Wait clusters put operation to complete */
	waitCompletion();

	/* Waiting for PE0 of each cluster to end */
	joinAll();

	/* Finalizes async server */
	async_master_finalize();

	//printf("Numero de amigos = %d\n", count_friends());
}