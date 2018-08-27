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

static Item *finishedTasks;
static Item *tasks;
static mppa_async_segment_t send_segments[NUM_CLUSTERS];
static mppa_async_segment_t receive_segments[NUM_CLUSTERS];

/* Parameters.*/
static int startnum;               /* Start number.      */
static int endnum;                 /* End number.        */
static int problemsize;            /* Total task size    */
static int avgtasksize;            /* Average task size. */
static int tasksize[NUM_CLUSTERS]; /* Task size.         */

void distributeTaskSizes(int _start, int _end) {
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

void sendWork() {
	int cont = 0;
	for (int i = 0; i < nclusters; i++) {
		char str_size[10];
		sprintf(str_size, "%d", tasksize[i]);
		char *args[2];
		args[0] = str_size;
		args[1] = NULL; 

		/* Creating segments to get tasks return values*/
		mppa_async_segment_create(&send_segments[i], i+1, &tasks[cont], tasksize[i]*sizeof(Item), 0, 0, NULL);

		/* Creating segment to receive values back */
		mppa_async_segment_create(&receive_segments[i], i+17, &finishedTasks[cont], tasksize[i]*sizeof(Item), 0, 0, NULL);
		
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
	tasks = smalloc(problemsize * sizeof(Item));
	finishedTasks = smalloc(problemsize * sizeof(Item));
	int aux = startnum;
	for (int i = 0; i < problemsize; i++)
		tasks[i].number = aux++; 

	/* Spawns clusters and creates segments */
	sendWork();

	inform_clusters_started();

	Item teste[3];
	mppa_async_get(&teste, &receive_segments[0], 0, 3 * sizeof(Item), NULL);
	mppa_async_fence(&receive_segments[0], NULL);
	
	for (int i = 0; i < 3; i++)
		printf("Cluster -> Put || IO -> Get ==> Teste = %d\n", teste[i].number);

	/* Waiting for PE0 of each cluster to end */
	for (int i = 0; i < nclusters; i++)
		join_slave(i);

	async_master_finalize();
}