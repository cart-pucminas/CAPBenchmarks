/* Kernel Includes */
#include <async_util.h>
#include <global.h>
#include <timer.h>
#include <util.h>
#include "../kernel.h"

/* C And MPPA Library Includes*/
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* Data exchange segments. */
static mppa_async_segment_t infos_seg;
static mppa_async_segment_t mask_seg;
static mppa_async_segment_t img_seg;
static mppa_async_segment_t output_seg;

/* Kernel variables. */
static char *img;          /* Input image.                     */
static char *output;       /* Output image.                    */ 
static int imgsize;        /* Dimension of image.              */
static int *mask;          /* Mask.                            */
static int masksize;       /* Dimension of mask.               */
static int numcorners = 0; /* Sum. of cornes.                  */
static int offset;         /* Intersection between two chunks. */
static int nchunks;        /* Nº of chunks to be processed.    */

/* Timing auxiliars */
static uint64_t start, end;

/* Statistics information from slaves. */
static struct message statistics[NUM_CLUSTERS];

/* Create segments for data and info exchange. */
static void create_segments() {
	createSegment(&signals_offset_seg, SIG_SEG_0, &sig_offsets, nclusters * sizeof(off64_t), 0, 0, NULL);
	createSegment(&infos_seg, MSG_SEG_0, &statistics, nclusters * sizeof(struct message), 0, 0, NULL);
	createSegment(&mask_seg, 5, mask, masksize * sizeof(int), 0, 0, NULL);
	createSegment(&img_seg, 6, img, imgsize * imgsize * sizeof(char), 0, 0, NULL);
	createSegment(&output_seg, 7, output, imgsize * imgsize * sizeof(char), 0, 0, NULL);
}


static void spawnSlaves() {
	start = timer_get();

	char str_masksize[10], str_offset[10], str_nchunks[10], str_nclusters[10];
	sprintf(str_masksize, "%d", masksize);
	sprintf(str_offset, "%d", offset);
	sprintf(str_nchunks, "%d", nchunks);
	sprintf(str_nclusters, "%d", nclusters);

	set_cc_signals_offset();

	/* Parallel spawning PE0 of cluster "i" */
	#pragma omp parallel for default(shared) num_threads(3)
	for (int i = 0; i < nclusters; i++) {
		char *args[6];
		args[0] = str_masksize;
		args[1] = str_offset;
		args[2] = str_nchunks;
		args[3] = str_nclusters;
		args[4] = str_cc_signals_offset[i];
		args[5] = NULL;

		spawn_slave(i, args);
	}
	end = timer_get();
	spawn = timer_diff(start, end);
}

/*
 * FAST corner detection.
 */
int fast(char *_img, char *_output, int _imgsize, int *_mask, int _masksize) {	
	img = _img;
	output = _output;
	imgsize = _imgsize;
	mask = _mask;
	masksize = _masksize;

	offset = (imgsize/CHUNK_SIZE)*MASK_RADIUS; 
	nchunks = (imgsize*imgsize)/(CHUNK_SIZE*CHUNK_SIZE);

	/* Initializes async server */
	async_master_start();

	/* Creates all necessary segments for data exchange */
	create_segments();

	/* Spawns all clusters. */
	spawnSlaves();

	/* Synchronize variable offsets between Master and Slaves. */
	get_slaves_signals_offset();

	/* Waiting for clusters answers. */
	for (int i = 0; i < nclusters; i++) {
		waitCondition(&cluster_signals[i], 0, MPPA_ASYNC_COND_GT, NULL);

		numcorners += cluster_signals[i];
		cluster_signals[i] = 0;
		
		/* Synchronization. */
		send_signal(i);
	}

	/* Wait all slaves statistics info. */
	wait_statistics();

	/* Set slaves statistics. */
	set_statistics(statistics);

	/* Waiting for PE0 of each cluster to end */
	join_slaves();

	/* Finalizes async server */
	async_master_finalize();
	
	return numcorners;
}