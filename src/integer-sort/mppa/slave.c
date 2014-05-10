/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * slave.c - slave process.
 */

#include <assert.h>
#include <mppa/osconfig.h>
#include <mppaipc.h>
#include <stdio.h>
#include "integer-sort.h"

/* Inter process communication. */
static int rank;    /* Process rank.   */
static int infd;    /* Input channel.  */
static int sync_fd; /* Sync file descriptor. */

/* Chunk of numbers. */
static int chunk[CHUNK_SIZE];

/*
 * Receives number from master process.
 */
static void receivechunk(void)
{	
	ssize_t count;
	
	count = mppa_read(infd, chunk, CHUNK_SIZE*sizeof(int));
	assert(count != -1);
}

/*
 * Opens NoC connectors.
 */
static void open_noc_connectors(void)
{
	char path[35];
	
	sync_fd = mppa_open("/mppa/sync/128:64", O_WRONLY);
	assert(sync_fd != -1);

	/* Open input channel. */
	sprintf(path, "/mppa/channel/%d:%d/128:%d", rank, rank + 1, rank + 1);
	infd = mppa_open(path, O_RDONLY);
	assert(infd != -1);
}

/*
 * Synchronizes with master process.
 */
static void sync_master(void)
{
	uint64_t mask;
	
	/* Synchronize with master. */
	mask = 1 << rank;
	assert(mppa_write(sync_fd, &mask, 8) == 8);
}

/*
 * Closes NoC connectors.
 */
static void close_noc_connectors(void)
{
	/* Close channels. */
	mppa_close(sync_fd);
	mppa_close(infd);
	
}

/*
 * Sorts a chunk of numbers.
 */
static void sort(void)
{
	int i, j; /* Loop index.       */
	int tmp;  /* Temporary number. */
	
	receivechunk();
	
	/* Sort. */
	for (i = 0; i < CHUNK_SIZE; i++)
	{
		for (j = i + 1; j < CHUNK_SIZE; j++)
		{
			if (chunk[i] > chunk[j])
			{
				tmp = chunk[j];
				chunk[j] = chunk[i];
				chunk[i] = tmp;
			}
		}
	}
}

int main(int argc, char **argv)
{
	((void)argc);
	
	rank = atoi(argv[0]);
	open_noc_connectors();
	sync_master();
	
	
	sort();
	
	close_noc_connectors();
	
	mppa_exit(0);
	return (0);
}
