#include <async_util.h>
#include <spawn_util.h>
#include <stdint.h> // Necessary for uint64_t.
#include <stdlib.h> // Necessary for malloc().
#include <global.h> // Necessary to use nclusters value.
#include <mppa_async.h>
#include <unistd.h> 

#include "master.h"

typedef struct {
    int number;
    int num;
    int den;
} Item;

#define MAX_TASK_SIZE 65536

static Item *finishedTasks;
static Item *task;

/* Parameters.*/
static int endnum;      /* Start number.      */
static int startnum;    /* End number.        */
static int *tasksize;   /* Task size.         */
static int avgtasksize; /* Average task size. */

void distributeTaskSizes(int _start, int _end) {
	startnum = _start;
	endnum = _end;

	int problemsize = (_end - _start + 1);

	if (problemsize > MAX_TASK_SIZE)
		problemsize = MAX_TASK_SIZE;

	avgtasksize = problemsize/nclusters;

	/* Each cluster receives a range of total task size */
	tasksize = (int *) malloc(nclusters * sizeof(int));
	
	/* Distribute task sizes. */
	for (int i = 0; i < nclusters; i++)
		tasksize[i] = (i + 1 < nclusters)?avgtasksize:problemsize-i*avgtasksize;
}

int friendly_numbers(int _start, int _end) {
	/* Try to do it with threads (FASTER) */
	distributeTaskSizes(_start, _end);

	async_master_init();

	spawn_slaves();

	async_master_start();

	/* Wait all distribution threads terminate their calculation before
	   send work to the clusters */
	//sendWork();

	join_slaves();

	async_master_finalize();
}