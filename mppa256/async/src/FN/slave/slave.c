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

/* Problem structure : number and abundances */
typedef struct {
    long int number; /* Number      */
    long int num; 	/* Numerator   */
    long int den; 	/* Denominator */
} Item;

#define MAX_TASK_SIZE 65536

/* Task sent by IO */
static Item task[MAX_TASK_SIZE];

/* Informations about the task */
static int offset;
static int tasksize; 

/* Timing auxiliars */
static uint64_t start, end;

/* Compute Cluster ID */
int cid;

/* Async Segments. */
static mppa_async_segment_t infos_seg;
static mppa_async_segment_t task_segment;

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

	sum = (n == 1) ? 1 : 1 + n;
	
	/* Compute sum of divisors. */
	for (factor = 2; factor <= maxD; factor++) {
		/* Divisor found. */
		if ((n % factor) == 0)
			sum += factor;
	}
	return (sum);
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

int main(__attribute__((unused)) int argc , const char **argv) {
	/* Initializes async client */
	async_slave_init();

	/* Problem information */
	cid = __k1_get_cluster_id();
	tasksize = atoi(argv[0]);
	offset = atoi(argv[1]);
	sigback_offset = (off64_t) atoll(argv[2]);

	/* Synchronization of timer */
	timer_init();

	/* Clone remote segments from IO */
	cloneSegment(&signals_offset_seg, SIG_SEG_0, 0, 0, NULL);
	cloneSegment(&infos_seg, MSG_SEG_0, 0, 0, NULL);
	cloneSegment(&task_segment, 3, 0, 0, NULL);

	/* Send io_signal offset to IO. */
	send_sig_offset();

	/* Get work from IO. */
	dataGet(task, &task_segment, offset, tasksize, sizeof(Item), NULL);

	/* Calculation of all numbers abundances */
	calc_abundances();

	/* Send abundances to IO. */
	dataPut(task, &task_segment, offset, tasksize, sizeof(Item), NULL);

	/* Put statistics in stats. segment on IO side. */
	send_statistics(&infos_seg);

	/* Finalizes async library and rpc client */
	async_slave_finalize();

	return 0;
}
