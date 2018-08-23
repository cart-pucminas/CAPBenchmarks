/* Kernel Includes */
#include <spawn_util.h>
#include <async_util.h>

/* C And MPPA Library Includes*/
#include <mppa_async.h>
#include <utask.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

/* Async Communicator. */
static mppa_async_segment_t image_segment;

/* Pointer to ddr buffer */
static void *image_ddr_ptr = NULL;

void* task(void *arg) {
	
}

int main(int argc , const char **argv) {
	async_slave_init();

	int cid = __k1_get_cluster_id();
	image_ddr_ptr = (void*)(uintptr_t)atoll(argv[0]);

	if (cid == 0)
		printf("Cluster 0 image_buffer %p\n", image_ddr_ptr);

	mppa_async_segment_clone(&image_segment, 10, 0, 0, 0);

	mppa_rpc_barrier_all();		/*  synchronize all PE0 of all booted cluster */

	int image_size = 2*sizeof(int);
	int *image_buffer = malloc(image_size);

	/* DDR -> Cluster */
	off64_t res;
	mppa_async_offset(&image_segment, image_ddr_ptr, &res);
	mppa_async_get(image_buffer, &image_segment, res, image_size, NULL);

	for (int i = 0; i < 2; i++) 
		printf("Image Buffer [0] = %d\n",image_buffer[i]);

	async_slave_finalize();
	return 0;
}
