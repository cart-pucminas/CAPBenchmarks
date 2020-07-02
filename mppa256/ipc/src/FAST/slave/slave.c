/*
 * Copyright(C) 2014 Alyson D. Pereira <alyson.deives@outlook.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <global.h>
#include <mppaipc.h>
#include <omp.h>
#include <timer.h>
#include <util.h>
#include <stdio.h>
#include <string.h>
#include "slave.h"

#include <ipc.h>

/* Timing statistics. */
uint64_t start;
uint64_t end;
uint64_t communication = 0;
uint64_t total = 0;

/* FAST parameters. */
static int aux_offset;
static int nchunks;     
static int masksize;
static int *mask;
static char *chunk;
static int corners[MAX_THREADS];
static int output[CHUNK_SIZE*CHUNK_SIZE];
static int corners_sum;

int nclusters;

/**
 * FAST corner detection.
 */
void fast(int offset, int n)
{
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


int main(int argc, char **argv)
{
	int msg,offset;

	timer_init();

	((void)argc);

	rank = atoi(argv[0]);	
	
	/* Setup interprocess communication. */
	open_noc_connectors();
	
	/* Receives filter mask. */
	data_receive(infd, &masksize, sizeof(int));
	mask = (int *) smalloc(masksize * sizeof(int));
	data_receive(infd, mask, sizeof(int)*masksize);

	data_receive(infd, &aux_offset, sizeof(int));
	chunk = (char *) smalloc((CHUNK_SIZE*CHUNK_SIZE) + (2* aux_offset + (CHUNK_SIZE*CHUNK_SIZE)));
    
    data_receive(infd, &nchunks, sizeof(int));
    data_receive(infd, &nclusters, sizeof(int));

    omp_set_num_threads(16);
    
	/* Process chunks. */
	int nchunk, chunk_size, i;
    for (nchunk = rank; nchunk < nchunks; nchunk += nclusters) 
    {
 		if (nchunk == nchunks - 1)
			chunk_size = (CHUNK_SIZE*CHUNK_SIZE) + (aux_offset * CHUNK_SIZE);
		else
			chunk_size = (CHUNK_SIZE*CHUNK_SIZE) + (2 * aux_offset * CHUNK_SIZE);

		data_receive(infd, chunk, chunk_size);		//Receives chunk

		memset(corners,0,MAX_THREADS*sizeof(int));
		memset(output,0,CHUNK_SIZE*CHUNK_SIZE*sizeof(char));

		start = timer_get();						
		(nchunk == 0) ? fast(0, chunk_size) : fast(aux_offset, chunk_size);
		end = timer_get();
		total += timer_diff(start, end);

		for (i = 0; i < MAX_THREADS; i++)
			corners_sum += corners[i];

		data_send(outfd, output, CHUNK_SIZE*CHUNK_SIZE*sizeof(char));
	}
	
	data_send(outfd, &corners_sum, sizeof(int));
	data_send(outfd, &total, sizeof(uint64_t));
	
	close_noc_connectors();
	mppa_exit(0);

	/* House keeping. */
	free(mask);
	free(chunk);

	return (0);
}
	
