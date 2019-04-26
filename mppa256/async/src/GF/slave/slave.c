/* Kernel Includes */
#include <async_util.h>
#include <timer.h>
#include <arch.h>
#include <util.h>
#include "../kernel.h"

/* C And MPPA Library Includes*/
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

/* Data exchange segments. */
static mppa_async_segment_t infos_seg;
static mppa_async_segment_t mask_seg;
static mppa_async_segment_t chunks_seg;

/* Kernel parameters. */
static int masksize;                             /* Mask dimension.              */
static int chunk_with_halo_size;     	         /* Chunk size including a halo. */
static double *mask;                             /* Mask.                        */
static unsigned char *chunk;                     /* Image input chunk.           */
static unsigned char newchunk[CHUNK_SIZE_SQRD];  /* Image output chunk.          */

/* Timing statistics auxiliars. */
static uint64_t start, end;

/* Individual Slave statistics. */
uint64_t total = 0;          /* Time spent on slave.    */
uint64_t communication = 0;  /* Time spent on comms.    */               
size_t data_put = 0;         /* Number of bytes put.    */
size_t data_get = 0;         /* Number of bytes gotten. */
unsigned nput = 0;           /* Number of put ops.      */
unsigned nget = 0;           /* Number of get ops.      */

/* Compute Cluster ID */
int cid;

static void clone_segments() {
	cloneSegment(&signals_offset_seg, SIG_SEG_0, 0, 0, NULL);
	cloneSegment(&infos_seg, MSG_SEG_0, 0, 0, NULL);
	cloneSegment(&mask_seg, 5, 0, 0, NULL);
	cloneSegment(&chunks_seg, 6, 0, 0, NULL);
}

/* Gaussian filter. */
void gauss_filter() {
	double pixel;
	int chunkI, chunkJ, maskI, maskJ;

	#pragma omp parallel default(shared) private(chunkI,chunkJ,maskI,maskJ,pixel)
	{
		#pragma omp for
		for (chunkI = 0; chunkI < CHUNK_SIZE; chunkI++) {
			for (chunkJ = 0; chunkJ < CHUNK_SIZE; chunkJ++) {
				pixel = 0.0;
				
				for (maskI = 0; maskI < masksize; maskI++) {
					for (maskJ = 0; maskJ < masksize; maskJ++)
						pixel += CHUNK(chunkI + maskI, chunkJ + maskJ) * MASK(maskI, maskJ);
				}
			   
				NEWCHUNK(chunkI, chunkJ) = (pixel > 255) ? 255 : (int)pixel;
			}
		}
	}
}

static void process_chuncks() {
	int msg = 0;              /* Msg type.                        */

	dataGet(mask, &mask_seg, 0, masksize*masksize, sizeof(double), NULL);

	/* Process chunks. */
	while (1) {
		waitCondition(&io_signal, 0, MPPA_ASYNC_COND_GT, NULL);
		msg = io_signal;
		io_signal = 0;

		if (msg == MSG_CHUNK) {
			dataGet(chunk, &chunks_seg, 0, chunk_with_halo_size * chunk_with_halo_size, sizeof(unsigned char), NULL);
			send_signal();

			start = timer_get();
			gauss_filter();
			end = timer_get();
			total += timer_diff(start, end);

			wait_signal();
			dataPut(newchunk, &chunks_seg, 0, CHUNK_SIZE_SQRD, sizeof(unsigned char), NULL);
			send_signal();		
		} else if (msg == MSG_DIE) {
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
	masksize = atoi(argv[0]);
	sigback_offset = (off64_t) atoll(argv[1]);
	chunk_with_halo_size = CHUNK_SIZE + masksize - 1;

	mask = smalloc(masksize * masksize * sizeof(double));
	chunk = (unsigned char *) smalloc(chunk_with_halo_size * chunk_with_halo_size * sizeof(unsigned char));

	/* Clones message exchange and io_signal segments */
	clone_segments();

	/* Send io_signal offset to IO. */
	send_sig_offset();

	/* Processing the chuncks. */
	process_chuncks();

	/* Put statistics in stats. segment on IO side. */
	send_statistics(&infos_seg);

	/* Finalizes async library and rpc client */
	async_slave_finalize();

	/* House keeping. */
	free(mask);
	free(chunk);

	return 0;
}