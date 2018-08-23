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
const mppa_async_segment_t GLOBAL_COMM;

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

	int image_size = 2*sizeof(int);
	int *image_buffer = malloc(image_size);
	char str_buff[50];
	sprintf(str_buff, "%lld", (uint64_t)(uintptr_t)image_buffer);
	char *args[2];
	args[0] = str_buff;
	args[1] = NULL; 

	/* Spawning PE0 of each cluster */
	for (int i = 0; i < nclusters; i++)
		spawn_slave(i, args);

	inform_clusters_started();

	async_master_start();

	printf("IO%d image_buffer %p\n", __k1_get_cluster_id()/192, image_buffer);

	for(int i = 0; i < 2; i++)
		image_buffer[i] = i;

	mppa_async_segment_t image_segment;
	mppa_async_segment_create(&image_segment, 10, image_buffer, image_size, 0, 0, NULL);

	/* Wait all distribution threads terminate their calculation before
	   send work to clusters */
	//sendWork();

	/* Waiting for PE0 of each cluster to end */
	for (int i = 0; i < nclusters; i++)
		join_slave(i);

	async_master_finalize();
}