/* Kernel Includes */
#include <async_util.h>
#include <problem.h>
#include <global.h> 
#include <timer.h>
#include <util.h>
#include "master.h"

/* C And MPPA Library Includes*/
#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h> 
#include <unistd.h> 

/* Timing statistics. */
uint64_t start;
uint64_t end;

#define MAX_TASK_SIZE 65536

/* Async Segments. */
static mppa_async_segment_t infos_segment;
static mppa_async_segment_t tasks_segment;

/* Slave offsets, tasks and work done */
static int offsets[NUM_CLUSTERS];
static Item tasks[MAX_TASK_SIZE];
static Info tasksFinished[NUM_CLUSTERS];

/* Total of friendly numbers */
static int friendlyNumbers = 0;

/* Parameters.*/
static int startnum;               /* Start number.      */
static int endnum;                 /* End number.        */
static int problemsize;            /* Total task size    */
static int avgtasksize;            /* Average task size. */
static int tasksize[NUM_CLUSTERS]; /* Task size.         */

static void distributeTaskSizes(int _start, int _end) {
	start = timer_get();

	startnum = _start;
	endnum = _end;

	problemsize = (_end - _start + 1);

	if (problemsize > MAX_TASK_SIZE)
		problemsize = MAX_TASK_SIZE;

	avgtasksize = problemsize/nclusters;
	
	/* Distribute task sizes. */
	for (int i = 0; i < nclusters; i++)
		tasksize[i] = (i + 1 < nclusters)?avgtasksize:problemsize-i*avgtasksize;

	end = timer_get();

	master += timer_diff(start, end);
}

static void setOffsets() {
	start = timer_get();

	int offset = 0;
	for (int i = 0; i < nclusters; i++) {
		offsets[i] = offset;
		offset += tasksize[i];
	}

	end = timer_get();

	master += timer_diff(start, end);
}

static void setTasks() {
	start = timer_get();

	int aux = startnum;
	for (int i = 0; i < problemsize; i++)
		tasks[i].number = aux++; 

	end = timer_get();

	master += timer_diff(start, end);
}

static void createSegments() {
	createSegment(&tasks_segment, 1, &tasks, problemsize * sizeof(Item), 0, 0, NULL);
	createSegment(&infos_segment, 2, &tasksFinished, nclusters * sizeof(Info), 0, 0, NULL);
}

static void sendWork() {
	int i;               /* Loop indexes. */

	start = timer_get();

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

	end = timer_get();

	spawn = timer_diff(start, end);
}

static void waitCompletion() {
	start = timer_get();

	for (int i= 0; i < nclusters; i++)
		waitCondition((long long *)&tasksFinished[i].slave, 0, MPPA_ASYNC_COND_GT, NULL);

	end = timer_get();

	nreceived += nclusters;
	data_received += nreceived * sizeof(Info);
	communication += timer_diff(start, end);
}

static void sumAll() {
	start = timer_get();

	for (int i = 0; i < nclusters; i++)
		friendlyNumbers += tasksFinished[i].parcial_sum;

	/* We calculated only the friendly pairs, so now
	   we have to multiply the result by 2 in order
	   to get all numbers that are friendly          */
	friendlyNumbers *= 2;

	end = timer_get();

	master += timer_diff(start, end);
}

static void joinAll() {
	for (int i = 0; i < nclusters; i++)
		join_slave(i);
}

static void setAllStatistics() {
	uint64_t comm_Sum = 0;
	uint64_t comm_Average = 0;
	for (int i = 0; i < nclusters; i++) {
		slave[i] = tasksFinished[i].slave;
		comm_Sum += tasksFinished[i].communication;
		data_sent += tasksFinished[i].data_sent;
		data_received += tasksFinished[i].data_received;
		nsent += tasksFinished[i].nsent;
		nreceived += tasksFinished[i].nreceived;
	}

	comm_Average = (uint64_t)comm_Sum/nclusters;
	communication += comm_Average;
}

int friendly_numbers(int _start, int _end) {
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

	/* Sum all partial sums */
	sumAll();

	/* Waiting for PE0 of each cluster to end */
	joinAll();

	/* Finalizes async server */
	async_master_finalize();

	/* Set statistics received by slaves */
	setAllStatistics();

	return friendlyNumbers;
}