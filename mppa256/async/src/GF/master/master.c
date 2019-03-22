/* Kernel Includes */
#include <util.h>
#include <timer.h>
#include <global.h>
#include <async_util.h>
#include "../kernel.h"

/* C And MPPA Library Includes*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Asynchronous segments */
static mppa_async_segment_t infos_seg;
static mppa_async_segment_t mask_seg;
static mppa_async_segment_t chunks_seg;
static mppa_async_segment_t newimg_seg;

/* Gaussian Filter. */
static unsigned char *img;       /* Input image.                    */
static unsigned char *newimg;    /* Output image.                   */ 
static int imgsize;              /* Dimension of image.             */
static double *mask;             /* Mask.                           */
static int masksize;             /* Dimension of mask.              */
static int chunk_with_halo_size; /* Chunk size including a halo.    */
static unsigned char *chunk;     /* chunks of an image per cluster. */

/* Workaround of the memory leak with the "huge" class. */
static unsigned int is_class_huge = 0;    /* Class of execution = "huge"?    */

/* Timing auxiliars */
static uint64_t start, end;

/* Statistics information from slaves. */
static struct message statistics[NUM_CLUSTERS];

/* Create segments for data and info exchange. */
static void create_segments() {
	createSegment(&signals_offset_seg, SIG_SEG_0, &sig_offsets, nclusters * sizeof(off64_t), 0, 0, NULL);
	createSegment(&infos_seg, MSG_SEG_0, &statistics, nclusters * sizeof(struct message), 0, 0, NULL);
	createSegment(&mask_seg, 5, mask, masksize * masksize * sizeof(double), 0, 0, NULL);
	createSegment(&chunks_seg, 6, chunk, chunk_with_halo_size * chunk_with_halo_size * sizeof(unsigned char), 0, 0, NULL);
	createSegment(&newimg_seg, 7, newimg, imgsize * imgsize * sizeof(unsigned char), 0, 0, NULL);
}

static void spawnSlaves() {
	start = timer_get();

	char str_masksize[10];
	sprintf(str_masksize, "%d", masksize);

	set_cc_signals_offset();

	/* Parallel spawning PE0 of cluster "i" */
	#pragma omp parallel for default(shared) num_threads(3)
	for (int i = 0; i < nclusters; i++) {
		char *args[3];
		args[0] = str_masksize;
		args[1] = str_cc_signals_offset[i];
		args[2] = NULL;

		spawn_slave(i, args);
	}
	end = timer_get();
	spawn = timer_diff(start, end);
}

static void process_chuncks() {
	int half = masksize/2;
	int nchunks = 0;

	int ii, jj;
	ii = 0;
	jj = 0;

	for (int i = half; i < imgsize - half; i += CHUNK_SIZE) {
		for (int j = half; j < imgsize - half; j += CHUNK_SIZE) {

			/* Build chunk. */
			for (int k = 0; k < chunk_with_halo_size; k++) 
				memcpy(&chunk[k * chunk_with_halo_size], &img[(i - half + k) * imgsize + j - half], chunk_with_halo_size * sizeof(unsigned char));

			/* Synchronization. */
			poke(mppa_async_default_segment(nchunks), sig_offsets[nchunks], MSG_CHUNK); 
			wait_signal(nchunks);

			/* Receives chunk without halo. */
			if (++nchunks == nclusters) {
				for (int ck = 0; ck < nchunks; ck++) {
					/* Waiting chunks. */
					send_signal(ck);
					wait_signal(ck);

					for (int k = 0; k < CHUNK_SIZE; k++) 
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
	for (int ck = 0; ck < nchunks; ck++) {
		/* Waiting chunks. */
		send_signal(ck);
		wait_signal(ck);

		/* Build chunk. */
		for (int k = 0; k < CHUNK_SIZE; k++)
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
	for (int i = 0; i < nclusters; i++) 
		send_signal(i);
}

/* Gaussian filter. */
void gauss_filter(unsigned char *img_, int imgsize_, double *mask_, int masksize_) {
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

	/* Initializes async server */
	async_master_start();

	/* Creates all necessary segments for data exchange */
	create_segments();

	/* Spawns all clusters. */
	spawnSlaves();

	/* Synchronize variable offsets between Master and Slaves. */
	get_slaves_signals_offset();

	/* Processing the chunks. */
	process_chuncks();

	/* Wait all slaves statistics info. */
	wait_statistics();

	/* Set slaves statistics. */
	set_statistics(statistics);

	/* Waiting for PE0 of each cluster to end */
	join_slaves();

	/* Finalizes async server */
	async_master_finalize();

	/* House keeping. */
	free(chunk);
	free(newimg);
}	