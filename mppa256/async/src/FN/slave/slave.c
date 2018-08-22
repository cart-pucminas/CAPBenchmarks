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
mppa_async_segment_t GLOBAL_COMM;


void* task(void *arg) {
	
}

int main(int argc , const char **argv) {
	async_slave_init();

	int startnum = atoi(argv[0]);
	int endnum = atoi(argv[1]);

	off64_t offset;
	mppa_async_segment_clone(&GLOBAL_COMM, 10, &offset, 2 * (sizeof(int)), NULL);

	printf("StartNum = %d and EndNum = %d\n", startnum, endnum);
	
	async_slave_finalize();
	return 0;
}
