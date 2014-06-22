/*
 * Copyright(C) 2014 Alyson D. Pereira <alyson.deives@outlook.com>, 
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
#include "master.h"

/*
 * FAST corner detection.
 */
int fast(char *img, int imgsize, int *mask, int masksize)
{	
	int i,j,k;             /* Loop indexes.     */ 
	size_t n;
	int n1,n2;            /* Bytes to send.    */
	int msg;             /* Message.          */
	int offset;			
	int nchunks;         /* Number of chunks. */
	int corners[MAX_THREADS] = {0};
	int numcorners = 0;
	
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
    n1 = ((CHUNK_SIZE)*(CHUNK_SIZE+4*MASK_RADIUS))*sizeof(char);
    n2 = ((CHUNK_SIZE)*(CHUNK_SIZE+2*MASK_RADIUS))*sizeof(char); //size of last chunk
    msg = MSG_CHUNK;
    nchunks = (imgsize*imgsize)/(CHUNK_SIZE*CHUNK_SIZE);
    printf("nchunks = %d\n",nchunks);
    for (i = 0; i < nchunks; i++)
    {		
		communication += data_send(outfd[j], &msg, sizeof(int));
		if(i == nchunks-1){
			communication += data_send(outfd[j], &n2, sizeof(int));
			communication += data_send(outfd[j], &img[i*(CHUNK_SIZE * (CHUNK_SIZE-2*MASK_RADIUS))],n2);
		}
		else{
			communication += data_send(outfd[j], &n1, sizeof(int));
			communication += data_send(outfd[j], &img[i*(CHUNK_SIZE * (CHUNK_SIZE-2*MASK_RADIUS))],n1);
		}
		offset = (i==0) ? 0 : 2*MASK_RADIUS;
		communication += data_send(outfd[j], &offset, sizeof(int));
		j++;
		
		/* 
		 * Slave processes are busy.
		 * So let's wait for results.
		 */
		if (j == nclusters)
		{
			for (/* NOOP */ ; j > 0; j--)
			{
				//communication += data_receive(infd[nclusters-j],&img[(nclusters-j)*CHUNK_SIZE*CHUNK_SIZE], n);
				communication += data_receive(infd[nclusters-j],&corners,MAX_THREADS*sizeof(int));
				printf("Master result: ");
				for(k=0;k<MAX_THREADS;k++){
					printf("%d ", corners[k]);
					numcorners += corners[k];
				}
				printf("\n");
			}
		}
	}
	
	/* Receive remaining results. */
	for (/* NOOP */ ; j > 0; j--)
	{
		//communication += data_receive(infd[j - 1],&img[(nchunks - j)*CHUNK_SIZE*CHUNK_SIZE], n);
		communication += data_receive(infd[j - 1],&corners,MAX_THREADS*sizeof(int));
		printf("Master result: ");
		for(k=0;k<MAX_THREADS;k++){
			printf("%d ", corners[k]);
			numcorners += corners[k];
		}
		printf("\n");
	}
	
	/* House keeping. */
	msg = MSG_DIE;
	for (i = 0; i < nclusters; i++)
		data_send(outfd[i], &msg, sizeof(int));
	join_slaves();
	close_noc_connectors();
	
	return numcorners;
}
