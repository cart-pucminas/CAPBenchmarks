/* Kernel Includes */
#include <spawn_util.h>
#include <async_util.h>
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

static Item *task;
static int tasksize; 
static int cid;

/* Async Communicator. */
static mppa_async_segment_t segment;

static void getWork() {
	mppa_async_segment_clone(&segment, cid+1, 0, 0, 0);

	/* DDR -> Cluster (Receive work) */
	mppa_async_get(task, &segment, 0, tasksize * sizeof(Item), NULL);
}

int main(int argc , const char **argv) {
	async_slave_init();

	cid = __k1_get_cluster_id();
	tasksize = atoi(argv[0]);

	task = smalloc(tasksize* sizeof(Item));

	getWork();

	/* DDR -> Cluster (Work is done) */
	//mppa_async_put(task, &segment, 0, tasksize * sizeof(Item), NULL);

	async_slave_finalize();
	return 0;
}
