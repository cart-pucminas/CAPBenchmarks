/* Kernel Includes */
#include <async_util.h>
#include <spawn_util.h>
#include <problem.h>
#include <global.h> 
#include "master.h"

/* C And MPPA Library Includes*/
#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h> 
#include <mppa_async.h>
#include <unistd.h> 
#include <mppa_power.h>

typedef struct {
	int number;
	int num;
	int den;
} Item;

#define MAX_TASK_SIZE 65536

static Item *finishedTasks;
static Item *task;

/* Parameters.*/
static int startnum;               /* Start number.      */
static int endnum;                 /* End number.        */
static int tasksize[NUM_CLUSTERS]; /* Task size.         */
static int avgtasksize;            /* Average task size. */

/* Async Communicator. */
mppa_async_segment_t GLOBAL_COMM;

void distributeTaskSizes(int _start, int _end) {
	startnum = _start;
	endnum = _end;

	int problemsize = (_end - _start + 1);

	if (problemsize > MAX_TASK_SIZE)
		problemsize = MAX_TASK_SIZE;

	avgtasksize = problemsize/nclusters;
	
	/* Distribute task sizes. */
	for (int i = 0; i < nclusters; i++)
		tasksize[i] = (i + 1 < nclusters)?avgtasksize:problemsize-i*avgtasksize;
}

int friendly_numbers(int _start, int _end) {
	/* Try to do it with pthreads (FASTER) */
	distributeTaskSizes(_start, _end);

	async_master_init();

	//spawn_slaves();

	char arg0[10];  /* Argument 0. */
	char arg1[10];	/* Argument 1. */
	char *args[3];  /* Arguments.  */

	sprintf(arg0, "%d", p->start);
	sprintf(arg1, "%d", p->end);
	args[0] = arg0;
	args[1] = arg1;
	args[2] = NULL;

	async_master_start();
	
	mppa_power_base_spawn(0, "cluster_bin", (const char **)args , NULL, MPPA_POWER_SHUFFLING_ENABLED);

	off64_t offset;
	mppa_async_segment_create (&GLOBAL_COMM, 10, &offset, 2 * sizeof(int), 0, 0, NULL);

	/* Wait all distribution threads terminate their calculation before
	   send work to clusters */
	//sendWork();

	int ret;
	mppa_power_base_waitpid(0, &ret, 0);
	//join_slaves();

	async_master_finalize();
}