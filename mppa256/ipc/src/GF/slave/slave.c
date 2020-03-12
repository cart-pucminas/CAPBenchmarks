/*
 * Copyright(C) 2014 Matheus M. Queiroz <matheus.miranda.queiroz@gmail.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <global.h>
#include <mppaipc.h>
#include <omp.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>
#include <stdio.h>
#include "../kernel.h"

/* Kernel parameters. */
static int masksize;                             /* Mask dimension.              */
static int chunk_with_halo_size;     	         /* Chunk size including a halo. */
static double *mask;                             /* Mask.                        */
static unsigned char *chunk;                     /* Image input chunk.           */
static unsigned char newchunk[CHUNK_SIZE_SQRD];  /* Image output chunk.          */

/* Timing statistics auxiliars. */
static uint64_t start, end;
uint64_t communication = 0;
uint64_t total = 0;

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

static void process_chunks() {
	int msg = 0;              /* Msg type.                        */

	data_receive(infd, mask, sizeof(double)*masksize*masksize);
	
	/* Process chunks. */
	while (1) {
		data_receive(infd, &msg, sizeof(int));

		if (msg == MSG_CHUNK) {
			data_receive(infd, chunk, chunk_with_halo_size * chunk_with_halo_size * sizeof(unsigned char));

			start = timer_get();
			gauss_filter();
			end = timer_get();
			total += timer_diff(start, end);

			data_send(outfd, &newchunk, CHUNK_SIZE_SQRD * sizeof(unsigned char));
		} else if (msg == MSG_DIE) {
			break;
		}
	}
}


int main(__attribute__((unused)) int argc, char **argv) {	
	/* Timer synchronization */
	timer_init();

	rank = atoi(argv[0]);	
	
	/* Setup interprocess communication. */
	open_noc_connectors();
	
	/* Receives filter mask.*/
	data_receive(infd, &masksize, sizeof(int));
	chunk_with_halo_size = CHUNK_SIZE + masksize - 1;
		
	mask = smalloc(masksize * masksize * sizeof(double));
	chunk = (unsigned char *) smalloc(chunk_with_halo_size * chunk_with_halo_size * sizeof(unsigned char));

	/* Processing the chuncks. */
	process_chunks();

	/* Sending time stats to IO. */
	data_send(outfd, &total, sizeof(uint64_t));

	/* House keeping. */
	free(mask);
	free(chunk);
	
	close_noc_connectors();
	mppa_exit(0);
	return (0);
}
	
