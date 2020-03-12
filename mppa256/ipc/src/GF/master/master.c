/*
 * Copyright(C) 2014 Matheus M. Queiroz <matheus.miranda.queiroz@gmail.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <arch.h>
#include <assert.h>
#include <global.h>
#include <inttypes.h>
#include <math.h>
#include <mppaipc.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>
#include "../kernel.h"


/* Gaussian Filter. */
static unsigned char *img;       /* Input image.                    */
static unsigned char *newimg;    /* Output image.                   */ 
static int imgsize;              /* Dimension of image.             */
static double *mask;             /* Mask.                           */
static int masksize;             /* Dimension of mask.              */
static int chunk_with_halo_size; /* Chunk size including a halo.    */
static unsigned char *chunk;     /* chunk to be sent to clusters X. */

/* Workaround of the memory leak with the "huge" class. */
static unsigned int is_class_huge = 0;    /* Class of execution = "huge"?    */

/* Timing auxiliars */
static uint64_t start, end;

/*
 * Wrapper to data_send(). 
 */
#define data_send(a, b, c)                   \
	{                                        \
		data_sent += c;                      \
		nsend++;                             \
		communication += data_send(a, b, c); \
	}                                        \

/*
 * Wrapper to data_receive(). 
 */
#define data_receive(a, b, c)                   \
	{                                           \
		data_received += c;                     \
		nreceive++;                             \
		communication += data_receive(a, b, c); \
	}                                           \

static void process_chunks() {
	int half = masksize/2;
	int nchunks = 0;
	int msg = MSG_CHUNK;

	int i, j, k, ii, jj, ck;
	ii = 0;
	jj = 0;

	for (i = half; i < imgsize - half; i += CHUNK_SIZE) {
		for (j = half; j < imgsize - half; j += CHUNK_SIZE) {

			/* Build chunk. */
			for (k = 0; k < chunk_with_halo_size; k++) 
				memcpy(&chunk[k * chunk_with_halo_size], &img[(i - half + k) * imgsize + j - half], chunk_with_halo_size * sizeof(unsigned char));

			/* Sending chunk to slave. */
			data_send(outfd[nchunks], &msg, sizeof(int));
			data_send(outfd[nchunks], chunk, chunk_with_halo_size * chunk_with_halo_size * sizeof(unsigned char));

			/* Receives chunk without halo. */
			if (++nchunks == nclusters) {
				for (ck = 0; ck < nchunks; ck++) {
					data_receive(infd[ck], chunk, CHUNK_SIZE_SQRD * sizeof(unsigned char));

					for (k = 0; k < CHUNK_SIZE; k++) 
						memcpy(&newimg[(ii + k)*imgsize + jj], &chunk[k * CHUNK_SIZE], CHUNK_SIZE * sizeof(unsigned char));
				
					jj += CHUNK_SIZE;
					if ((jj+masksize-1) == imgsize) {
						jj = 0;
						ii += CHUNK_SIZE;
						if ((is_class_huge) && (ii == 16384)) 
							ii = 0;
					}
				}
				nchunks = 0;
			}
		}
	}

	/* Get remaining chunks. */
	for (ck = 0; ck < nchunks; ck++) {
		data_receive(infd[ck], chunk, CHUNK_SIZE_SQRD * sizeof(unsigned char));

		/* Build chunk. */
		for (k = 0; k < CHUNK_SIZE; k++)
			memcpy(&newimg[(ii + k)*imgsize + jj], &chunk[k * CHUNK_SIZE], CHUNK_SIZE * sizeof(unsigned char));

		jj += CHUNK_SIZE;
		if ((jj+masksize-1) == imgsize) {
			jj = 0;
			ii += CHUNK_SIZE;
			if ((is_class_huge) && (ii == 16384)) 
			ii = 0;
		}
	}

	/* Releasing slaves. */
	msg = MSG_DIE;
	for (i = 0; i < nclusters; i++) 
		data_send(outfd[i], &msg, sizeof(int));
}

/* Gaussian filter. */
void gauss_filter(unsigned char *img_, int imgsize_, double *mask_, int masksize_) {
	int i;

	/* Setup parameters. */
	img = img_;
	mask = mask_;
	imgsize = imgsize_;
	masksize = masksize_;
	chunk_with_halo_size = CHUNK_SIZE + masksize - 1;
	chunk = (unsigned char *) smalloc(chunk_with_halo_size * chunk_with_halo_size * sizeof(unsigned char));

	if (imgsize == 32782) {
		/* We will construct the new img on a array of with width = 32782 and 
		   high = 16394. That's because we don't have enough memory to allocate
		   an array of 32782 x 32782. So the new img will be construct in 
		   this 32782 x 16394 array which will give us the correct time result as
		   if we are constructing the new img on an 32782 x 32782, but won't give 
		   the blurred img.  */
		newimg = (unsigned char *) scalloc(32782*16394, sizeof(unsigned char));
		is_class_huge = 1;
	} else { 
		newimg = (unsigned char *) scalloc(imgsize * imgsize, sizeof(unsigned char));
	}
	
	/* Spawns all clusters. */
	open_noc_connectors();
	start = timer_get();
	spawn_slaves();
	end = timer_get();
	spawn = timer_diff(start, end);

	 /* Send mask. */
	for (i = 0; i < nclusters; i++) {
		data_send(outfd[i], &masksize, sizeof(int));
		data_send(outfd[i], mask, sizeof(double)*masksize*masksize);
	}

	/* Processing the chunks. */
	process_chunks();
	
	/* Waiting for PE0 of each cluster to end */
	join_slaves();
	close_noc_connectors();

	/* House keeping. */
	free(chunk);
	free(newimg);
}
