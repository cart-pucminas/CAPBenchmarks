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

typedef struct {
	long int number;
	long int numerator;
	long int denominator;
} Item;

/* Timing statistics. */
uint64_t start;
uint64_t end;
uint64_t communication = 0;
uint64_t total = 0;

static Item *task;
static int tasksize; 
static off64_t offset;

int cid;

/* Async Segments. */
static mppa_async_segment_t time_segment;
static mppa_async_segment_t task_segment;

/*
 * Computes the Greatest Common Divisor of two numbers.
 */
static int gcd(int a, int b)
{
	int c;

  /* Compute greatest common divisor. */
	while (a != 0)
	{
		c = a;
		a = b%a;
		b = c;
	}

	return (b);
}

/*
 * Some of divisors.
 */
static int sumdiv(int n)
{
	int sum;    /* Sum of divisors. */
	int factor; /* Working factor.  */
	
	sum = 1 + n;
	
	/* Compute sum of divisors. */
	for (factor = 2; factor < n; factor++)
	{
		/* Divisor found. */
		if ((n%factor) == 0)
			sum += factor;
	}
	
	return (sum);
}

static void cloneSegments() {
	mppa_async_segment_clone(&task_segment, 1, 0, 0, NULL);
	mppa_async_segment_clone(&time_segment, 2, 0, 0, NULL);
}

static void getWork() {
	mppa_async_get(task, &task_segment, offset, tasksize * sizeof(Item), NULL);
}

static void syncNumbers() {
	mppa_async_put(task, &task_segment, offset, tasksize * sizeof(Item), NULL);
	mppa_async_put(&total, &time_segment, cid * sizeof(uint64_t), sizeof(uint64_t), NULL);
}

void friendly_numbers() {
	int n; /* Divisor.      */
	int i; /* Loop indexes. */

	/* Compute abundances. */
	#pragma omp parallel for private(i, n) default(shared)
	for (i = 0; i < tasksize; i++) 
	{		
		task[i].numerator = sumdiv(task[i].number);
		task[i].denominator = i;

		n = gcd(task[i].numerator, task[i].denominator);
		task[i].numerator /= n;
		task[i].denominator /= n;
	}
}

int main(int argc , const char **argv) {
	/* Initializes async client */
	async_slave_init();
	
	cid = __k1_get_cluster_id();
	tasksize = atoi(argv[0]);
	offset = atoll(argv[1]);

	task = smalloc(tasksize * sizeof(Item));

	/* Clone remote segments from IO */
	cloneSegments();

	/* Get tasks from the remote segment */
	getWork();

	/* Synchronization of timer */
	timer_init();

	start = timer_get();
	friendly_numbers();
	end = timer_get();

	/* Total slave time */
	total = timer_diff(start, end);
	
	/* Send back the finished tasks to IO */
	syncNumbers();

	/* Finalizes async client */
	async_slave_finalize();
	return 0;
}
