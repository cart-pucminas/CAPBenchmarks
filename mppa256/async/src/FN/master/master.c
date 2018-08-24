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
static int *buffer;

/* Parameters.*/
static int startnum;     /* Start number.      */
static int endnum;       /* End number.        */
static int problemsize;  /* Total task size    */
static int avgtasksize;  /* Average task size. */
static int *tasksize;    /* Task size.         */

void distributeTaskSizes(int _start, int _end) {
	tasksize = (int*) malloc(nclusters * sizeof(int));
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

int friendly_numbers(int _start, int _end) {
	/* Try to do it with pthreads (FASTER) */
	distributeTaskSizes(_start, _end);

	async_master_init();

	buffer = malloc(problemsize * sizeof(int));

	for (int i = 0; i < problemsize; i++)
		buffer[i] = i;

	int cont = 0;
	for (int i = 0; i < nclusters; i++) {
		char str_size[10];
		char str_buff[50];
		sprintf(str_buff, "%lld", (uint64_t)(uintptr_t)(&buffer[cont]));
		sprintf(str_size, "%d", tasksize[i]);
		char *args[3];
		args[0] = str_size;
		args[1] = str_buff;
		args[2] = NULL; 

		/* Spawning PE0 of cluster i*/
		spawn_slave(i, args);
		if (i+1 < nclusters)
			cont += tasksize[i+1];
	}

	inform_clusters_started();

	async_master_start();

	mppa_async_segment_t segments[nclusters];

	__builtin_k1_wpurge();
	__builtin_k1_fence();

	cont = 0;
	for (int i = 0; i < nclusters; i++) {
		mppa_async_segment_create(&segments[i], i+1, &buffer[cont], tasksize[i]*sizeof(int), 0, 0, NULL);
		if (i+1 < nclusters)
			cont += tasksize[i+1];
	}

	/* Wait all distribution threads terminate their calculation before
	   send work to clusters */
	//sendWork();

	int a = 0;
	mppa_async_get(&a, &segments[0], (off64_t)0*sizeof(int), sizeof(int), NULL);

	printf("Tentando Cluster -> Put -> IO -> Get ==> Buff[0] = %d\n", a);

	/* Waiting for PE0 of each cluster to end */
	for (int i = 0; i < nclusters; i++)
		join_slave(i);

	async_master_finalize();
}