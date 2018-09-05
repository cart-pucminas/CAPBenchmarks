/* Kernel Includes */
#include <spawn_util.h>
#include <async_util.h>
#include <problem.h>
#include <global.h>
#include <timer.h>
#include <util.h>

/* C And MPPA Library Includes*/
#include <mppa_async.h>
#include <utask.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    long int number; /* Number      */
    long int num; 	/* Numerator   */
    long int den; 	/* Denominator */
} Item;

/* Timing statistics and parcial sum result */
typedef struct {
	size_t data_sent;
	size_t data_received;
	unsigned nsent;
	unsigned nreceived;
	uint64_t slave;
	uint64_t communication;
	int parcial_sum;
} Info;

#define MAX_TASK_SIZE 65536

/* Timing statistics aux */
uint64_t start;
uint64_t end;

/* Task sent by IO */
static Item task[MAX_TASK_SIZE];

/* All tasks after each Cluster complete their 
   individual task. Necessary for comparison  */
static Item allTasks[MAX_TASK_SIZE];

/* Information to send back to IO */
static Info finishedTask = {0, 0, 0, 0, 0, 0, 0};

/* Informations about the task */
static int offset;
static int tasksize; 
static int problemsize;
static int parcial_friendly_sum = 0;

/* Compute Cluster ID */
int cid;

/* Async Segments. */
static mppa_async_segment_t task_segment;
static mppa_async_segment_t info_segment;

/*
 * Computes the Greatest Common Divisor of two numbers.
 */
static int gcd(int a, int b) {
	int mod;

  /* Compute greatest common divisor. */
	while (b != 0)
	{
		mod = a % b;
		a = b;
		b = mod;
	}

	return a;
}

/*
 * Some of divisors. (Algorithm considering n >= 2)
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
	mppa_async_segment_clone(&info_segment, 2, 0, 0, NULL);
}

static void getWork() {
	start = timer_get();

	mppa_async_get(task, &task_segment, offset * sizeof(Item), tasksize * sizeof(Item), NULL);

	end = timer_get();

	finishedTask.nreceived += tasksize-1;
	finishedTask.data_received += tasksize * sizeof(Item);
	finishedTask.communication += timer_diff(start, end);	
}

static void syncAbundances() {
	start = timer_get();

	mppa_async_put(task, &task_segment, offset * sizeof(Item), tasksize * sizeof(Item), NULL);

	mppa_rpc_barrier_all();

	mppa_async_fence(&task_segment, NULL);

	mppa_async_get(allTasks, &task_segment, 0, problemsize * sizeof(Item), NULL);

	end = timer_get();

	finishedTask.nsent += tasksize;
	finishedTask.data_sent += tasksize * sizeof(Item);
	finishedTask.nreceived += problemsize;
	finishedTask.data_received += problemsize * sizeof(Item);
	finishedTask.communication += timer_diff(start, end);
}

static void calc_abundances() {
	int n; /* Divisor.      */
	int i; /* Loop indexes. */
	
	start = timer_get();

	/* Compute abundances. */
	#pragma omp parallel for private(i, n) default(shared)
	for (i = 0; i < tasksize; i++) {		
		task[i].num = sumdiv(task[i].number);
		task[i].den = task[i].number;

		n = gcd(task[i].num, task[i].den);

		if (n != 0) {
			task[i].num /= n;
			task[i].den /= n;
		}
	}

	end = timer_get();

	/* Total slave time */
	finishedTask.slave += timer_diff(start, end);
}


static void countFriends() {
	int i; /* Loop indexes. */

	start = timer_get();

	#pragma omp parallel for private(i) default(shared) reduction(+: parcial_friendly_sum)
	for (i = offset; i < offset + tasksize; i++) {
		for (int j = 0; j < i; j++) {
			if ((allTasks[i].num == allTasks[j].num) && (allTasks[i].den == allTasks[j].den))
				finishedTask.parcial_sum++;
		}
	}

	end = timer_get();

	/* Total slave time */
	finishedTask.slave += timer_diff(start, end);
}

static void sendFinishedTask() {
	finishedTask.nsent++;
	finishedTask.data_sent += sizeof(Info);

	mppa_async_put(&finishedTask, &info_segment, cid * sizeof(Info), sizeof(Info), NULL);
}

int main(int argc , const char **argv) {
	/* Initializes async client */
	async_slave_init();

	/* Problem information */
	cid = __k1_get_cluster_id();
	problemsize = atoi(argv[0]);
	tasksize = atoi(argv[1]);
	offset = atoi(argv[2]);

	/* Synchronization of timer */
	timer_init();

	/* Clone remote segments from IO */
	cloneSegments();

	/* Get tasks from the remote segment */
	getWork();

	/* Calculation of all numbers abundances */
	calc_abundances();

	/* Synchronization of all abundances */
	syncAbundances();

	/* Count parcial sum of friendly pairs */
	countFriends();

	/* Sends back to IO parcial sums, exec and comm time */
	sendFinishedTask();

	/* Finalizes async library and rpc client */
	async_slave_finalize();

	return 0;
}
