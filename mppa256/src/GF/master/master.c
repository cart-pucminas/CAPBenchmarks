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
#include "master.h"

/*
 * Gaussian filter.
 */
void gauss_filter(unsigned char *img, int imgsize, double *mask, int masksize)
{	
	int i,j;             /* Loop indexes.     */ 
	size_t n;            /* Bytes to send.    */
	int msg;             /* Message.          */
	int nchunks;         /* Number of chunks. */
	
	open_noc_connectors();
	spawn_slaves();
	sync_slaves();
	
	 /* Send mask. */
    n = sizeof(double)*masksize*masksize;	
	for (i = 0; i < nthreads; i++)
	{
		communication += data_send(outfd[i], &masksize, sizeof(int));
		communication += data_send(outfd[i], mask, n);
	}
    
    /* Process image in chunks. */
    j = 0; n = CHUNK_SIZE*CHUNK_SIZE; msg = MSG_CHUNK;
    nchunks = (imgsize*imgsize)/(CHUNK_SIZE*CHUNK_SIZE);
    for (i = 0; i < nchunks; i++)
    {		
		communication += data_send(outfd[j], &msg, sizeof(int));
		communication += data_send(outfd[j], &img[i*(CHUNK_SIZE*CHUNK_SIZE)],n);
		
		j++;
		
		/* 
		 * Slave processes are busy.
		 * So let's wait for results.
		 */
		if (j == nthreads)
		{
			for (/* NOOP */ ; j > 0; j--)
			{
				communication += data_receive(infd[nthreads-j],
								   &img[(nthreads-j)*CHUNK_SIZE*CHUNK_SIZE], n);
			}
		}
	}
	
	/* Receive remaining results. */
	for (/* NOOP */ ; j > 0; j--)
	{
		communication += data_receive(infd[j - 1], 
								&img[(nchunks - j)*CHUNK_SIZE*CHUNK_SIZE], n);
	}
	
	/* House keeping. */
	msg = MSG_DIE;
	for (i = 0; i < nthreads; i++)
		data_send(outfd[i], &msg, sizeof(int));
	join_slaves();
	close_noc_connectors();
}
