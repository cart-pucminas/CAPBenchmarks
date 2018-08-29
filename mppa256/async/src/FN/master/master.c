/* Kernel Includes */
#include <async_util.h>
#include <spawn_util.h>
#include <problem.h>
#include <global.h> 
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

static mppa_async_segment_t segment;

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

static void setTasks() {
	int aux = startnum;
	for (int i = 0; i < problemsize; i++)
		tasks[i].number = aux++; 
}

static void sendWork() {
	/* Creating segments to get tasks return values*/
	mppa_async_segment_create(&segment, 1, &tasks, problemsize * sizeof(Item), 0, 0, NULL);

	off64_t offset = 0;
	for (int i = 0; i < nclusters; i++) {
		char str_size[10], str_offset[50];
		sprintf(str_size, "%d", tasksize[i]);
		sprintf(str_offset, "%lld", offset);
		char *args[3];
		args[0] = str_size;
		args[1] = str_offset;
		args[2] = NULL; 
		
		/* Spawning PE0 of cluster i*/
		spawn_slave(i, args);
		
		offset += tasksize[i] * sizeof(Item);
	}
}

static void syncNumbers() {
	mppa_async_evalcond(&tasks[problemsize-1].num, 0, MPPA_ASYNC_COND_NE, 0);
}

static void joinAll() {
	for (int i = 0; i < nclusters; i++)
		join_slave(i);
}

static void test() {
	int aux = 0;
	for (int i = 0; i < nclusters; i++) {
		for (int j = aux; j < aux + 10; j++)
			printf("Task[%d] = %d || %d || %d\n", i, tasks[j].number, tasks[j].num, tasks[j].den);
		aux += tasksize[i];
	}
}

int friendly_numbers(int _start, int _end) {
	/* Intervals to each cluster */
	distributeTaskSizes(_start, _end);

	/* Initializes async server */
	async_master_init();
	async_master_start();

	/* Initializes tasks number values */
	setTasks();

	/* Spawns clusters and creates segments */
	sendWork();

	/* Wait clusters work */
	syncNumbers();

	test();

	/* Waiting for PE0 of each cluster to end */
	joinAll();

	async_master_finalize();

	fflush(stdout);
}