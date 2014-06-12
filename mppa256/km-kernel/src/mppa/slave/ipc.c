/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <mppaipc.h>

/* Inter process communication. */
int rank;           /* Process rank.         */
int infd;           /* Input channel.        */
int outfd;          /* Output channel.       */
static int sync_fd; /* Sync file descriptor. */


/*
 * Synchronizes with master process.
 */
void sync_master(void)
{
	uint64_t mask;
	
	/* Synchronize with master. */
	mask = 1 << rank;
	assert(mppa_write(sync_fd, &mask, 8) == 8);
}

/*
 * Opens NoC connectors.
 */
void open_noc_connectors(void)
{
	char path[35];
	
	sync_fd = mppa_open("/mppa/sync/128:64", O_WRONLY);
	assert(sync_fd != -1);

	/* Open input channel. */
	sprintf(path, "/mppa/channel/%d:%d/128:%d", rank, rank + 1, rank + 1);
	infd = mppa_open(path, O_RDONLY);
	assert(infd != -1);
	sprintf(path, "/mppa/channel/128:%d/%d:%d", rank + 33, rank, rank + 33);
	outfd = mppa_open(path, O_WRONLY);
	assert(outfd != -1);
}

/*
 * Closes NoC connectors.
 */
void close_noc_connectors(void)
{
	/* Close channels. */
	mppa_close(sync_fd);
	mppa_close(infd);
	mppa_close(outfd);
}
