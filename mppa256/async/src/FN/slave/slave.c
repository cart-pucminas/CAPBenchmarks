/* Kernel Includes */
#include <async_util.h>
#include <problem.h>
#include <global.h>
#include <timer.h>
#include <util.h>

/* C And MPPA Library Includes*/
#include <utask.h>
#include <stdlib.h>

/* Individual Slave statistics. */
uint64_t total = 0;         /* Time spent on slave.    */
uint64_t communication = 0; /* Time spent on comms.    */               
size_t data_put = 0;        /* Number of bytes put.    */
size_t data_get = 0;        /* Number of bytes gotten. */
unsigned nput = 0;          /* Number of items put.    */
unsigned nget = 0;          /* Number of items gotten. */

/* Statistics and partial sum results */
typedef struct {
	size_t data_put;
	size_t data_get;
	unsigned nput;
	unsigned nget;
	uint64_t slave;
	uint64_t communication;
	int partial_sum;
} Info;

/* Problem structure : number and abundances */
typedef struct {
    long int number; /* Number      */
    long int num; 	/* Numerator   */
    long int den; 	/* Denominator */
} Item;

#define MAX_TASK_SIZE 65536

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
static int partial_friendly_sum = 0;

/* Timing auxiliars */
static uint64_t start, end;

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
	cloneSegment(&task_segment, 1, 0, 0, NULL);
	cloneSegment(&info_segment, 2, 0, 0, NULL);
}

static void getWork() {
	dataGet(task, &task_segment, offset, tasksize, sizeof(Item), NULL);
}

static void syncAbundances() {
	dataPut(task, &task_segment, offset, tasksize, sizeof(Item), NULL);

	slave_barrier();

	waitAllOpCompletion(&task_segment, NULL);

	dataGet(allTasks, &task_segment, 0, problemsize, sizeof(Item), NULL);
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
	total += timer_diff(start, end);
}


static void countFriends() {
	int i; /* Loop indexes. */

	start = timer_get();

	#pragma omp parallel for private(i) default(shared) reduction(+: partial_friendly_sum)
	for (i = offset; i < offset + tasksize; i++) {
		for (int j = 0; j < i; j++) {
			if ((allTasks[i].num == allTasks[j].num) && (allTasks[i].den == allTasks[j].den))
				partial_friendly_sum++;
		}
	}

	end = timer_get();

	/* Total slave time */
	total += timer_diff(start, end);
}

static void setFinishedTask() {
	finishedTask.data_put = data_put;
	finishedTask.data_get = data_get;
	finishedTask.nput = nput;
	finishedTask.nget = nget;
	finishedTask.slave = total;
	finishedTask.communication = communication;
	finishedTask.partial_sum = partial_friendly_sum;
}

static void sendFinishedTask() {
	dataPut(&finishedTask, &info_segment, cid, 1, sizeof(Info), NULL);
}

int main(__attribute__((unused)) int argc , const char **argv) {
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

	/* Count partial sum of friendly pairs */
	countFriends();

	/* Set statistics and partial sum in finishedTask */
	setFinishedTask();

	/* Send statistics to IO. */
	sendFinishedTask();

	/* Finalizes async library and rpc client */
	async_slave_finalize();

	return 0;
}
