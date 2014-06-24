/*
 * Copyright(C) 2014 Alyson D. Pereira <alyson.deives@outlook.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <arch.h>
#include <assert.h>
#include <global.h>
#include <inttypes.h>
#include <stdint.h>
#include <math.h>
#include <mppaipc.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>
#include <util.h>
#include <stdio.h>
#include "master.h"

/*
 * FAST corner detection.
 */
int fast(char *img, char *output, int imgsize, int *mask, int masksize)
{	
	int i,j,k;             /* Loop indexes.     */ 
	size_t n;
	int msg;             /* Message.          */
	int offset;			
	int nchunks;         /* Number of chunks. */
	int corners[MAX_THREADS] = {0};
	int numcorners = 0;
	
	uint64_t start,end;
	
	open_noc_connectors();
	spawn_slaves();
	sync_slaves();
	
	 /* Send mask. */
    	n = sizeof(int)*masksize;	
	for (i = 0; i < nclusters; i++)
	{
		communication += data_send(outfd[i], &masksize, sizeof(int));
		communication += data_send(outfd[i], mask, n);
	}
    
    	/* Process image in chunks. */
    	j = 0; 
     	msg = MSG_CHUNK;
    	nchunks = (imgsize*imgsize)/(CHUNK_SIZE*CHUNK_SIZE);
	
    	for (i = 0; i < nchunks; i++)
   	{		
		communication += data_send(outfd[j], &msg, sizeof(int));
		offset = (imgsize/CHUNK_SIZE)*MASK_RADIUS;
		
		if(i == nchunks-1){
			int start = i*(CHUNK_SIZE * CHUNK_SIZE)- offset*CHUNK_SIZE;
			int end = (CHUNK_SIZE*CHUNK_SIZE)+(offset*CHUNK_SIZE);
			communication += data_send(outfd[j], &end, sizeof(int));
			communication += data_send(outfd[j], &img[start],end*sizeof(char));
		}
		else{
			int start = i*(CHUNK_SIZE * CHUNK_SIZE)- offset*CHUNK_SIZE;
			int end = (CHUNK_SIZE*CHUNK_SIZE)+(2*offset*CHUNK_SIZE);
			communication += data_send(outfd[j], &end, sizeof(int));
			communication += data_send(outfd[j], &img[start],end*sizeof(char));
		}
		if(i == 0){
			communication += data_send(outfd[j], &i, sizeof(int));
		}
		else{
			communication += data_send(outfd[j], &offset, sizeof(int));
		}
		
		j++;
		
		/* 
		 * Slave processes are busy.
		 * So let's wait for results.
		 */
		if (j == nclusters)
		{
			for (/* NOOP */ ; j > 0; j--)
			{
				communication += data_receive(infd[nclusters-j],&corners,MAX_THREADS*sizeof(int));
				communication += data_receive(infd[nclusters-j],&output[(nclusters - j)*CHUNK_SIZE*CHUNK_SIZE], CHUNK_SIZE*CHUNK_SIZE*sizeof(char));
				
				start=timer_get();
				for(k=0;k<MAX_THREADS;k++){
					numcorners += corners[k];
				}
				end=timer_get();
				master += timer_diff(start,end);
			}
		}
	}
	
	/* Receive remaining results. */
	for (/* NOOP */ ; j > 0; j--)
	{
		communication += data_receive(infd[j - 1],&corners,MAX_THREADS*sizeof(int));
		communication += data_receive(infd[j - 1],&output[(nchunks - j)*CHUNK_SIZE*CHUNK_SIZE], CHUNK_SIZE*CHUNK_SIZE*sizeof(char)); 
		
		start=timer_get();
		for(k=0;k<MAX_THREADS;k++){
			numcorners += corners[k];
		}
		end=timer_get();
		master += timer_diff(start,end);
	}
	
	/* House keeping. */
	msg = MSG_DIE;
	for (i = 0; i < nclusters; i++)
		data_send(outfd[i], &msg, sizeof(int));
	join_slaves();
	close_noc_connectors();
	
	return numcorners;
}
