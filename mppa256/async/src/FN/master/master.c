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

#define MAX_TASK_SIZE 65536

/* Async Segments. */
static mppa_async_segment_t times_segment;
static mppa_async_segment_t tasks_segment;
static mppa_async_segment_t parcialsFSum_segement;

/* Slave tasks and offsets */
static float tasks[MAX_TASK_SIZE];
static int offsets[NUM_CLUSTERS];

/* Parcials sums of friendly number pairs */
static int parcialSums[NUM_CLUSTERS];

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
	int offset = 0;
	for (int i = 0; i < nclusters; i++) {
		offsets[i] = offset;
		offset += tasksize[i];
	}
}

static void setTasks() {
	int aux = startnum;
	for (int i = 0; i < problemsize; i++)
		tasks[i] = aux++; 
}

static void createSegments() {
	mppa_async_segment_create(&tasks_segment, 1, &tasks, problemsize * sizeof(float), 0, 0, NULL);
	mppa_async_segment_create(&times_segment, 2, &slave, nclusters * sizeof(uint64_t), 0, 0, NULL);
	mppa_async_segment_create(&parcialsFSum_segement, 3, &parcialSums, nclusters * sizeof(int), 0, 0, NULL);
}

static void sendWork() {
	int i;               /* Loop indexes. */
	omp_set_dynamic(0);
	#pragma omp parallel for private(i) default(shared) num_threads(3)
	for (i = 0; i < nclusters; i++) {
		char str_prb_size[10], str_size[10], str_offset[10];
		sprintf(str_prb_size, "%d", problemsize);
		sprintf(str_size, "%d", tasksize[i]);
		sprintf(str_offset, "%d", offsets[i]);
		char *args[4];
		args[0] = str_prb_size;
		args[1] = str_size;
		args[2] = str_offset;
		args[3] = NULL; 
		
		/* Spawning PE0 of cluster i*/
		spawn_slave(i, args);
	}
	omp_set_dynamic(1);
}

static void waitCompletion() {
	for (int i= 0; i < nclusters; i++)
		mppa_async_evalcond((long long *)&slave[i], 0, MPPA_ASYNC_COND_GT, 0);
	mppa_async_fence(&parcialsFSum_segement, NULL);
}

static void joinAll() {
	for (int i = 0; i < nclusters; i++)
		join_slave(i);
}

static void test() {
	int aux = 0;
	for (int i = 0; i < nclusters; i++) {
		for (int j = aux; j < aux + 10; j++) {
			printf("Task[%d] = %f\n", j, tasks[j]);
			fflush(stdout);
		}
		aux += tasksize[i];
	}
}

static void test2() {
	for (int i = 0; i < nclusters; i++) {
		printf("SumPart[%d] = %d\n", i, parcialSums[i]);
		fflush(stdout);
	}
}

void friendly_numbers(int _start, int _end) {
	/* Intervals to each cluster */
	distributeTaskSizes(_start, _end);

	/* Initializes async server */
	async_master_start();

	/* Initializes tasks number values */
	setTasks();

	/* Set clusters tasks offsets */
	setOffsets();

	/* Creating segments to get tasks returns values*/
	createSegments();

	/* Spawns clusters and send them work */
	sendWork();

	/* Waits slaves parcial sum */
	waitCompletion();

	/* Waiting for PE0 of each cluster to end */
	joinAll();

	test();
	test2();

	/* Finalizes async server */
	async_master_finalize();
}