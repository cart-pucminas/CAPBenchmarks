/* Kernel Includes */
#include <spawn_util.h>
#include <async_util.h>
#include <global.h>
#include <timer.h>
#include <util.h>

/* C And MPPA Library Includes*/
#include <mppa_async.h>
#include <utask.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_TASK_SIZE 65536

/* Timing statistics. */
uint64_t start;
uint64_t end;
uint64_t communication = 0;
uint64_t total = 0;

static float task[MAX_TASK_SIZE];
static float allAbundances[MAX_TASK_SIZE];

static int tasksize; 
static int problemsize;
static int parcial_friendly_sum = 0;

static int offset;

int cid;

/* Async Segments. */
static mppa_async_segment_t time_segment;
static mppa_async_segment_t task_segment;
static mppa_async_segment_t parcSum_segment;

/* Async events */
static mppa_async_event_t getAllAbundances_event;

/*
 * Some of divisors.
 */
static int sumdiv(int n) {
	int sum;    /* Sum of divisors.     */
	int factor; /* Working factor.      */
	int maxD; 	/* Max divisor before n */

	maxD = (int)n/2;
	sum = 1 + n;
	
	/* Compute sum of divisors. */
	for (factor = 2; factor <= maxD; factor++) {
		/* Divisor found. */
		if ((n % factor) == 0)
			sum += factor;
	}
	return (sum);
}

static void cloneSegments() {
	mppa_async_segment_clone(&task_segment, 1, 0, 0, NULL);
	mppa_async_segment_clone(&time_segment, 2, 0, 0, NULL);
	mppa_async_segment_clone(&parcSum_segment, 3, 0, 0, NULL);
}

static void getWork() {
	mppa_async_get(task, &task_segment, offset * sizeof(float), tasksize * sizeof(float), NULL);
}

static void syncAbundances() {
	mppa_async_put(task, &task_segment, offset * sizeof(float), tasksize * sizeof(float), NULL);
	mppa_rpc_barrier_all();
	mppa_async_fence(&task_segment, NULL);
	mppa_async_get(allAbundances, &task_segment, 0, problemsize * sizeof(float), &getAllAbundances_event);
}

void calc_abundances() {
	int i; /* Loop indexes. */

	start = timer_get();

	/* Compute abundances. */
	#pragma omp parallel for private(i) default(shared)
	for (i = 0; i < tasksize; i++) 	
		task[i] =  (float)sumdiv(task[i])/task[i];

	end = timer_get();

	/* Total slave time */
	total += timer_diff(start, end);
}


static void countFriends() {
	int i; /* Loop indexes. */

	mppa_async_event_wait(&getAllAbundances_event);

	start = timer_get();

	#pragma omp parallel for private(i) default(shared) reduction(+: parcial_friendly_sum)
	for (i = offset; i < offset + tasksize; i++) {
		for (int j = i+1; j < problemsize; j++) {
			if (allAbundances[i] == allAbundances[j])
				parcial_friendly_sum++;
		}
	}

	end = timer_get();

	/* Total slave time */
	total += timer_diff(start, end);
}

static void sendFinishedTask() {
	mppa_async_put(&parcial_friendly_sum, &parcSum_segment, cid * sizeof(int), sizeof(int), NULL);
	mppa_async_put(&total, &time_segment, cid * sizeof(uint64_t), sizeof(uint64_t), NULL);
}

int main(int argc , const char **argv) {
	/* Initializes async client */
	async_slave_init();
	
	/* Problem information */
	cid = __k1_get_cluster_id();
	problemsize = atoi(argv[0]);
	tasksize = atoi(argv[1]);
	offset = atoi(argv[2]);

	/* Clone remote segments from IO */
	cloneSegments();

	/* Get tasks from the remote segment */
	getWork();

	/* Synchronization of timer */
	timer_init();

	/* Calculation of all numbers abundances */
	calc_abundances();

	/* Synchronization of all abundances */
	syncAbundances();

	/* Count parcial sum of friendly pairs */
	countFriends();

	/* Sends back to IO parcial sums and exec time */
	sendFinishedTask();

	async_slave_finalize();
	return 0;
}
