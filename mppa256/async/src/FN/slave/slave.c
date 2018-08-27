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

/* Async Communicator. */
static mppa_async_segment_t work_segment;
static mppa_async_segment_t finished_tasks_segment;

/* Pointer to ddr buffer */
static void *adress = NULL;

int main(int argc , const char **argv) {
	async_slave_init();
	int cid = __k1_get_cluster_id();
	tasksize = atoi(argv[0]);

	mppa_async_segment_clone(&work_segment, cid+1, 0, 0, 0);
	mppa_async_segment_clone(&finished_tasks_segment, cid+17, 0, 0, 0);

	task = smalloc(tasksize* sizeof(Item));

	/* DDR -> Cluster */
	mppa_async_get(task, &work_segment, 0, tasksize * sizeof(Item), NULL);

	/* Cluster -> DDR */
	// OBS : TESTING
	
	Item a[3];
	for (int i = 0; i < 3; i++)
		a[i].number = 19;
	mppa_async_put(&a, &finished_tasks_segment, 0, 3 * sizeof(Item), NULL);
	mppa_async_fence(&finished_tasks_segment, NULL);

	async_slave_finalize();
	return 0;
}
