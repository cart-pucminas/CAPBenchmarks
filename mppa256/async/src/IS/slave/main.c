/* Kernel Includes */
#include <async_util.h>
#include <global.h>
#include <message.h>
#include <timer.h>
#include <util.h>

/* C And MPPA Library Includes*/
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <omp.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

/* Max size of mini-bucket. */
#define MINIBUCKET_SIZE 262144


/* Tests if partial sorting was successful. */
#define test_partial_order(array, n) {			\
	for (int l = 0; l < ((n)-1); l++) 			\
		assert(((array)[l]) <= ((array)[l+1]));	\
}

/* Tests if there's a negative number in the array */
#define test_non_negatives(array, n) {		\
	for (int l = 0; l < (n); l++) 			\
		assert(((array)[l]) >= 0);			\
}

/* Asynchronous segments */
static mppa_async_segment_t infos_seg;
static mppa_async_segment_t minibs_seg;

/* Timing statistics auxiliars. */
static uint64_t start, end;

/*  Array block. */
struct  {
	int size;                      /* Size of block. */
	int elements[MINIBUCKET_SIZE]; /* Elements.      */
} block;

/* Sorts an array of numbers. */
extern void sort2power(int *array, int size, int chunksize);

/* Individual Slave statistics. */
uint64_t total = 0;          /* Time spent on slave.    */
uint64_t communication = 0;  /* Time spent on comms.    */               
size_t data_put = 0;         /* Number of bytes put.    */
size_t data_get = 0;         /* Number of bytes gotten. */
unsigned nput = 0;           /* Number of put ops.      */
unsigned nget = 0;           /* Number of get ops.      */

/* Compute Cluster ID & num. of clusters being used. */
int cid;

static void clone_segments() {
	cloneSegment(&signals_offset_seg, SIG_SEG_0, 0, 0, NULL);
	cloneSegment(&infos_seg, MSG_SEG_0, 0, 0, NULL);
	cloneSegment(&minibs_seg, 3, 0, 0, NULL);
}

static void work() {
	struct message *msg; /* Message.              */
	int sizeAux;         /* Auxiliary block size. */

	/* Slave life. */
	while(1) {
		wait_signal();
		msg = message_get(&infos_seg, cid, NULL);

		if (msg->type == SORTWORK) {
			/* Extract message information. */
			block.size = msg->u.sortwork.size;
			message_destroy(msg);

			/* Receive part of the array. */
			dataGet(block.elements, &minibs_seg, cid * MINIBUCKET_SIZE, block.size, sizeof(int), NULL);

			/* Sorting... */
			start = timer_get();

			sizeAux = block.size;
			while (sizeAux % NUM_THREADS != 0) {
				block.elements[sizeAux] = INT_MAX;
				sizeAux++;
			}

			/* Sorts minibucket. */
			sort2power(block.elements, sizeAux, sizeAux/NUM_THREADS);
			end = timer_get();
			total += timer_diff(start, end);

			// test_partial_order(block.elements, sizeAux);
			// test_non_negatives(block.elements, sizeAux);

			/* Send data sorted. */
			dataPut(block.elements, &minibs_seg, cid * MINIBUCKET_SIZE, block.size, sizeof(int), NULL);
			
			/* Message is ready signal. */
			send_signal();
		} else {
			message_destroy(msg);
			break;
		}
	}
}

int main(__attribute__((unused))int argc, char **argv) {
	/* Initializes async client */
	async_slave_init();

	/* Timer synchronization */
	timer_init();

	/* Util information for the problem. */
	cid = __k1_get_cluster_id();
	sigback_offset = (off64_t) atoll(argv[0]);

	/* Clones message exchange and io_signal segments */
	clone_segments();

	/* Send io_signal offset to IO. */
	send_sig_offset();

	/* Slave life. */
	work();

	/* Put statistics in stats. segment on IO side. */
	send_statistics(&infos_seg);

	/* Finalizes async library and rpc client */
	async_slave_finalize();

	return 0;
}