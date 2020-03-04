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

#include <ipc.h>
#include "master.h"

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
	int numcorners = 0;
	
	uint64_t start,end;
	
	open_noc_connectors();
	start = timer_get();
	spawn_slaves();
	end = timer_get();
	spawn = timer_diff(start,end);

	offset = (imgsize/CHUNK_SIZE)*MASK_RADIUS;

    /* Process image in chunks. */
    j = 0; 
    msg = MSG_CHUNK;
    nchunks = (imgsize*imgsize)/(CHUNK_SIZE*CHUNK_SIZE);

	/* Send mask. */
    n = sizeof(int)*masksize;	
	for (i = 0; i < nclusters; i++)
	{
		data_send(outfd[i], &masksize, sizeof(int));
		data_send(outfd[i], mask, n);
		data_send(outfd[i], &offset, sizeof(int));
		data_send(outfd[i], &nchunks, sizeof(int));
		data_send(outfd[i], &nclusters, sizeof(int));
	}

	
	for (i = 0; i < nchunks; i++)
   	{		
		if(i == nchunks-1) {
			int start = i*(CHUNK_SIZE * CHUNK_SIZE)- offset*CHUNK_SIZE;
			int end = (CHUNK_SIZE*CHUNK_SIZE)+(offset*CHUNK_SIZE);
			data_send(outfd[j], &img[start],end*sizeof(char));
		} else {
			int start = i*(CHUNK_SIZE * CHUNK_SIZE)- offset*CHUNK_SIZE;
			int end = (CHUNK_SIZE*CHUNK_SIZE)+(2*offset*CHUNK_SIZE);
			data_send(outfd[j], &img[start],end*sizeof(char));
		}
		
		j++;
		
		/* 
		 * Slave processes are busy.
		 * So let's wait for results.
		 */
		if (j == nclusters)
		{
			for (/* NOOP */ ; j > 0; j--)
				data_receive(infd[nclusters-j],&output[(nclusters - j)*CHUNK_SIZE*CHUNK_SIZE], CHUNK_SIZE*CHUNK_SIZE*sizeof(char));
		}
	}
	
	/* Receives remaining results. */
	for (/* NOOP */ ; j > 0; j--)
		data_receive(infd[j - 1],&output[(nchunks - j)*CHUNK_SIZE*CHUNK_SIZE], CHUNK_SIZE*CHUNK_SIZE*sizeof(char)); 
	
	/* Receives sum of corners. */
	int part_sum = 0;
	for (i = 0; i < nclusters; i++) {
		data_receive(infd[i], &part_sum, sizeof(int));
		numcorners += part_sum;
	}

	join_slaves();
	close_noc_connectors();
	
	return numcorners;
}
