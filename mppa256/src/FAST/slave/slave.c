/*
 * Copyright(C) 2014 Alyson D. Pereira <alyson.deives@outlook.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <global.h>
#include <mppaipc.h>
#include <omp.h>
#include <timer.h>
#include <util.h>
#include "slave.h"

/* Timing statistics. */
uint64_t start;
uint64_t end;
uint64_t communication = 0;
uint64_t total = 0;

/* FAST parameters. */
static int masksize;
static int mask[MASK_SIZE];
static char chunk[(CHUNK_SIZE*CHUNK_SIZE)+IMG_SIZE*MASK_RADIUS];
static int output[MAX_THREADS];
//static int points[MAX_THREADS];
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
				//points[omp_get_thread_num()]++;
				z = 0;
				while(z<16){
					accumBrighter = 0;
					accumDarker = 0;
					for(r = 0;r<9;r++){
						x = i + mask[((r+z) * 2) + 0];
						y = j + mask[((r+z) * 2) + 1];

						//if(x >=0 && x < (CHUNK_SIZE) && y >=0 && y < (CHUNK_SIZE-2*MASK_RADIUS)){
						if (x >= 0 && y>=0 && ((y*CHUNK_SIZE + x) < n)){
							imagePixel = chunk[y * (CHUNK_SIZE) + x];
							if(imagePixel >= (centralPixel+THRESHOLD) && accumBrighter == 0){
								accumDarker++;
							}
							else if (imagePixel<=(centralPixel-THRESHOLD) && accumDarker == 0){
								accumBrighter++;
							}
							else{
								goto not_a_corner;
							}
						}
						//else{
							//printf("Out of bound: %d %d %d %d %d\n",i,j,x,y,(y*CHUNK_SIZE + x));
						//}
					}
					if(accumBrighter == 9 || accumDarker == 9){
						output[omp_get_thread_num()]++;
						z = 16;
						//printf("Corner detected\n");
					}
not_a_corner:		z=z+r;			
				}
			}
		}
	}
}


int main(int argc, char **argv)
{
	int msg,offset;
	int k;
	timer_init();
	int n;

	((void)argc);
	
    total = 0;

	rank = atoi(argv[0]);	
	
	/* Setup interprocess communication. */
	open_noc_connectors();
	sync_master();
	
	/* Receives filter mask.*/
	data_receive(infd, &masksize, sizeof(int));
	data_receive(infd, mask, sizeof(int)*masksize);
    
    omp_set_num_threads(16);
    
    
	/* Process chunks. */
    while (1)
	{
		data_receive(infd, &msg, sizeof(int));

		/* Parse message. */
		switch (msg)
		{
			case MSG_CHUNK:
				data_receive(infd, &n, sizeof(int)); 		//Receives size of chunk (includes halo)
				data_receive(infd, chunk, n);				//Receives chunk
				data_receive(infd, &offset, sizeof(int));	//Receive offset
				
				memset(output,0,MAX_THREADS*sizeof(int));
				//memset(points,0,MAX_THREADS*sizeof(int));
				
				start = timer_get();
				//printf("Processing...Offset = %d Size= %d\n", offset,n);

				
				fast(offset, n);
				
				/*printf("Slave result: ");
				for(k=0;k<MAX_THREADS;k++){
					printf("%d ", output[k]);
				}
				printf("\n\n");
				
				printf("Points Slave result: ");
				for(k=0;k<MAX_THREADS;k++){
					printf("%d ", points[k]);
				}
				printf("\n\n");
				*/
				
				end = timer_get();
				total += timer_diff(start, end);
				data_send(outfd, output, MAX_THREADS*sizeof(int));
				//data_send(outfd, points, MAX_THREADS*sizeof(int));
				
				break;
			
			default:
				goto out;
		}
	}

out:
	
	data_send(outfd, &total, sizeof(uint64_t));
	
	close_noc_connectors();
	mppa_exit(0);
	return (0);
}
	
