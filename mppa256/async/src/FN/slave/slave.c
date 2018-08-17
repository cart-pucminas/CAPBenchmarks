#include <spawn_util.h>
#include <async_util.h>
#include <mppa_async.h>
#include <utask.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

void* task(void *arg) {
	
}

int main(int argc , const char **argv) {
	async_slave_init();

	/* don't do anything before library initialisation */

	int cid = __k1_get_cluster_id();

	int i;
	printf("Cluster %d Spawned\n", cid);

	off64_t offset;
	mppa_async_malloc(MPPA_ASYNC_DDR_0, 4 * sizeof(utask_t), &offset, NULL);
	
	// Mudar para constante determinando n de PE'S a serem usad.
	utask_t *tasks = malloc(4 * sizeof(utask_t));

	mppa_async_get(tasks, MPPA_ASYNC_DDR_0, offset, 4 * sizeof(utask_t), NULL);

	for (i = 0; i < 16; i++)
		utask_create(&tasks[i], NULL, task, NULL);

	for (i = 0; i < 16; i++)
		utask_join(tasks[i], NULL);

	async_slave_finalize();
	return 0;
}
