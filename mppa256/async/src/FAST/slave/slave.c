/* Kernel Includes */
#include <async_util.h>
#include <global.h>
#include <timer.h>
#include <util.h>
#include "../kernel.h"

/* C And MPPA Library Includes*/
#include <omp.h>
#include <stdio.h>
#include <string.h>

/* Asynchronous segments */
static mppa_async_segment_t infos_seg;
static mppa_async_segment_t mask_seg;
static mppa_async_segment_t img_seg;
static mppa_async_segment_t output_seg;

/* FAST parameters. */
static int masksize;
static int *mask;
static char *chunk;
static int corners[MAX_THREADS];
static int output[CHUNK_SIZE_SQRD];
static int corners_sum;

/* From master. */
static int aux_offset;
static int nchunks;       

/* Timing statistics auxiliars. */
static uint64_t start, end;

/* Individual Slave statistics. */
uint64_t total = 0;          /* Time spent on slave.    */
uint64_t communication = 0;  /* Time spent on comms.    */               
size_t data_put = 0;         /* Number of bytes put.    */
size_t data_get = 0;         /* Number of bytes gotten. */
unsigned nput = 0;           /* Number of put ops.      */
unsigned nget = 0;           /* Number of get ops.      */

/* Compute Cluster ID & num. of clusters being used. */
int cid;
int nclusters;

static void clone_segments() {
	cloneSegment(&signals_offset_seg, SIG_SEG_0, 0, 0, NULL);
	cloneSegment(&infos_seg, MSG_SEG_0, 0, 0, NULL);
	cloneSegment(&mask_seg, 5, 0, 0, NULL);
	cloneSegment(&img_seg, 6, 0, 0, NULL);
	cloneSegment(&output_seg, 7, 0, 0, NULL);
}

/* FAST corner detection. */
void fast(int offset, int n) {
	int i,j,r,z,x,y;
	char accumBrighter, accumDarker;
	char imagePixel,centralPixel;
	
	#pragma omp parallel default(shared) private(imagePixel,centralPixel,i,j,z,r,x,y,accumBrighter,accumDarker)
	{
		#pragma omp for
		for (j = offset; j<CHUNK_SIZE+offset; j++){
			for (i = 0; i<CHUNK_SIZE; i++){
				centralPixel = chunk[j*CHUNK_SIZE + i];
				
				z = 0;
				while(z<16){
					accumBrighter = 0;
					accumDarker = 0;
					for(r = 0;r<12;r++){
						x = i + mask[((r+z) * 2) + 0];
						y = j + mask[((r+z) * 2) + 1];
						
						if (x >= 0 && y>=0 && ((y*CHUNK_SIZE + x) < n)){
							imagePixel = chunk[y * (CHUNK_SIZE) + x];
							if(imagePixel >= (centralPixel+THRESHOLD) ){
								if( accumBrighter == 0){
									accumDarker++;
								}
								else{ //Sequence is not contiguous
									z += r - 1;
									goto not_a_corner;
								}
							}
							else if (imagePixel<=(centralPixel-THRESHOLD) ){
								if (accumDarker == 0){
									accumBrighter++;
								}
								else{ //Sequence is not contiguous
									z += r - 1;
									goto not_a_corner;
								}
							}
							else{ //Actual pixel is inside threshold interval 
								z += r;								
								goto not_a_corner;
							}
						}
					}
					if(accumBrighter == 12 || accumDarker == 12){
						corners[omp_get_thread_num()]++;
						output[(j-offset)*CHUNK_SIZE + i] = 1;
						z = 16;
					}
not_a_corner:				z++;			
				}
			}
		}
	}
}

/* Process chunks. */
static void process_chuncks() {
	/* Auxiliary variables. */
	int chunk_start = 0;
	int chunk_size;
	int output_start;

	corners_sum = 0; 

	dataGet(mask, &mask_seg, 0, masksize, sizeof(int), NULL);

	for (int nchunk = cid; nchunk < nchunks; nchunk += nclusters) {
		if (nchunk > 0)
			chunk_start = (nchunk * CHUNK_SIZE_SQRD) - (aux_offset * CHUNK_SIZE);

		if (nchunk == nchunks - 1)
			chunk_size = CHUNK_SIZE_SQRD + (aux_offset * CHUNK_SIZE);
		else
			chunk_size = CHUNK_SIZE_SQRD + (2 * aux_offset * CHUNK_SIZE);

		dataGet(chunk, &img_seg, chunk_start, chunk_size, sizeof(char), NULL);

		memset(corners,0,MAX_THREADS*sizeof(int));
		memset(output,0,CHUNK_SIZE_SQRD*sizeof(char));

		start = timer_get();
		(nchunk == 0) ? fast(0, chunk_size) : fast(aux_offset, chunk_size);
		end = timer_get();
		total += timer_diff(start, end);

		for (int i = 0; i < MAX_THREADS; i++)
			corners_sum += corners[i];

		output_start = nchunk  * CHUNK_SIZE_SQRD;
			
		/* Sending back results. */
		dataPut(output, &output_seg, output_start, CHUNK_SIZE_SQRD, sizeof(char), NULL);
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
	aux_offset = atoi(argv[1]);
	nchunks = atoi(argv[2]);
	nclusters = atoi(argv[3]);
	sigback_offset = (off64_t) atoll(argv[4]);

	mask = (int *) smalloc(masksize * sizeof(int));
	chunk = (char *) smalloc(CHUNK_SIZE_SQRD + (2* aux_offset + CHUNK_SIZE_SQRD));

	/* Clones message exchange and io_signal segments */
	clone_segments();

	/* Send io_signal offset to IO. */
	send_sig_offset();

	/* Processing the chuncks. */
	process_chuncks();

	/* Send back partial sum of corners */
	poke(MPPA_ASYNC_DDR_0, sigback_offset, (long long) corners_sum);

	/* Synchronization. */
	wait_signal();

	/* Put statistics in stats. segment on IO side. */
	send_statistics(&infos_seg);

	/* Finalizes async library and rpc client */
	async_slave_finalize();

	/* House keeping. */
	free(mask);
	free(chunk);
	
	return 0;
}