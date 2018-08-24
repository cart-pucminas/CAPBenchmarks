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
static mppa_async_segment_t sub_buff_seg;

/* Pointer to ddr buffer */
static void *sub_buff_ddr_ptr = NULL;

void* task(void *arg) {
	
}

int main(int argc , const char **argv) {
	async_slave_init();

	int cid = __k1_get_cluster_id();
	int sub_buff_size = atoi(argv[0]);
	sub_buff_ddr_ptr = (void*)(uintptr_t)atoll(argv[1]);

	mppa_rpc_barrier_all();	

	mppa_async_segment_clone(&sub_buff_seg, cid+1, 0, 0, 0);

	mppa_rpc_barrier_all();	

	int *sub_buff = malloc(sub_buff_size * sizeof(int));

	/* DDR -> Cluster */
	off64_t offset;
	mppa_async_offset(&sub_buff_seg, sub_buff_ddr_ptr, &offset);
	mppa_async_get(sub_buff, &sub_buff_seg, offset, sub_buff_size * sizeof(int), NULL);

	/* Cluster -> DDR */
	// OBS : TESTING
	if(cid == 0) {
		int a = 200;
		sub_buff[0] = 100;
		printf("Cluster SubBuff[0] = %d\n", sub_buff[0]);
		mppa_async_put(&a, &sub_buff_seg, offset, sizeof(int), NULL);
		mppa_async_get(sub_buff, &sub_buff_seg, offset, sizeof(int), NULL);
		printf("Cluster SubBuff[0] = %d\n", sub_buff[0]);
	}

	async_slave_finalize();
	return 0;
}
