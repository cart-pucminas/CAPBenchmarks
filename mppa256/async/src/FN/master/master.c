/* Kernel Includes */
#include <async_util.h>
#include <problem.h>
#include <global.h> 
#include <timer.h>
#include <util.h>

/* C And MPPA Library Includes*/
#include <stdio.h>
#include <stdint.h> 

/* Items to be sent to slaves */
typedef struct {
    int number; /* Number      */
    int num; 	/* Numerator   */
    int den; 	/* Denominator */
} Item;

/* Timing statistics. */
static uint64_t start;
static uint64_t end;

#define MAX_TASK_SIZE 65536

/* Async Segments. */
static mppa_async_segment_t infos_seg;
static mppa_async_segment_t tasks_segment;

/* Statistics information from slaves. */
static struct message statistics[NUM_CLUSTERS];

/* Slave offsets, tasks and work done */
static int offsets[NUM_CLUSTERS];
static Item tasks[MAX_TASK_SIZE];

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
	createSegment(&signals_offset_seg, SIG_SEG_0, &sig_offsets, nclusters * sizeof(off64_t), 0, 0, NULL);
	createSegment(&infos_seg, MSG_SEG_0, &statistics, nclusters * sizeof(struct message), 0, 0, NULL);
	createSegment(&tasks_segment, 3, &tasks, problemsize * sizeof(Item), 0, 0, NULL);
}

static void spawnSlaves() {
	start = timer_get();

	char str_prb_size[10];
	sprintf(str_prb_size, "%d", problemsize);

	set_cc_signals_offset();

	#pragma omp parallel for default(shared) num_threads(3)
	for (int i = 0; i < nclusters; i++) {
		char str_size[10], str_offset[10];
		sprintf(str_size, "%d", tasksize[i]);
		sprintf(str_offset, "%d", offsets[i]);
		char *args[5];
		args[0] = str_prb_size;
		args[1] = str_size;
		args[2] = str_offset;
		args[3] = str_cc_signals_offset[i];
		args[4] = NULL; 

		/* Spawning PE0 of cluster i*/
		spawn_slave(i, args);
	}

	end = timer_get();

	spawn = timer_diff(start, end);
}

static void sumFriendlyNumbers() {
	int i;

	start = timer_get();

	#pragma omp parallel for private(i) default(shared) num_threads(3) reduction(+: friendlyNumbers)
	for (i = 0; i < problemsize; i++) {
		for (int j = i + 1; j < problemsize; j++) {
			if (tasks[i].num == tasks[j].num && tasks[i].den == tasks[j].den)
				friendlyNumbers++;
		}
	}

	end = timer_get();

	master += timer_diff(start, end);
}

/* Sum all friendly numbers. */
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

	/* Spawns all "nclusters" clusters */
	spawnSlaves();

	/* Synchronize variable offsets between Master and Slaves. */
	get_slaves_signals_offset();

	/* Wait all slaves statistics info. */
	wait_statistics();

	/* Sum all friendly numbers. */
	sumFriendlyNumbers();

	/* Set slaves statistics. */
	set_statistics(statistics);

	/* Waiting for PE0 of each cluster to end */
	join_slaves();

	/* Finalizes async server */
	async_master_finalize();

	return friendlyNumbers;
}