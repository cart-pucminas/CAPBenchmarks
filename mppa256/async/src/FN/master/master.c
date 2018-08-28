/* Kernel Includes */
#include <async_util.h>
#include <spawn_util.h>
#include <problem.h>
#include <global.h> 
#include <infos.h>
#include <util.h>
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

static Item tasks[MAX_TASK_SIZE];

static mppa_async_segment_t segments[NUM_CLUSTERS];

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

static void sendWork() {
	int cont = 0;
	for (int i = 0; i < nclusters; i++) {
		char str_size[10];
		sprintf(str_size, "%d", tasksize[i]);
		char *args[2];
		args[0] = str_size;
		args[1] = NULL; 

		/* Creating segments to get tasks return values*/
		mppa_async_segment_create(&segments[i], i+1, &tasks[cont], tasksize[i]*sizeof(Item), 0, 0, NULL);
		
		/* Spawning PE0 of cluster i*/
		spawn_slave(i, args);
		
		if (i+1 < nclusters)
			cont += tasksize[i+1];
	}
}

int friendly_numbers(int _start, int _end) {
	/* Try to do it with pthreads (FASTER) */
	distributeTaskSizes(_start, _end);

	async_master_init();

	async_master_start();

	/* Initialize tasks and finishedTasks*/
	int aux = startnum;
	for (int i = 0; i < problemsize; i++)
		tasks[i].number = aux++; 

	/* Spawns clusters and creates segments */
	sendWork();

	inform_clusters_started();

	/* Work is done
	for (int i = 0; i < nclusters; i ++)
		mppa_async_get(tasks, &segments[i], 0, tasksize[i] * sizeof(Item), NULL);

	printf("Cluster -> Put || IO -> Get ==> Teste = %d\n", tasks[0].number);
	printf("Cluster -> Put || IO -> Get ==> Teste = %d\n", tasks[1].number);
	fflush(stdout);*/

	/* Waiting for PE0 of each cluster to end */
	for (int i = 0; i < nclusters; i++)
		join_slave(i);

	async_master_finalize();
}