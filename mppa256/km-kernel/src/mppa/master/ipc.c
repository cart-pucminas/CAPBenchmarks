/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <assert.h>
#include <arch.h>
#include <global.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <util.h>

/* Interprocess communication. */
int infd[NUM_CLUSTERS];               /* Input channels.       */
int outfd[NUM_CLUSTERS];              /* Output channels.      */
static mppa_pid_t pids[NUM_CLUSTERS]; /* Processes IDs.        */
static int sync_fd;                   /* Sync file descriptor. */

/*
 * Spwans slave processes.
 */
void spawn_slaves(void)
{
	int i;          /* Loop index. */
	char arg0[4];   /* Argument 0. */
	char *args[2];  /* Arguments.  */

	/* Spawn slaves. */
	args[1] = NULL;
	for (i = 0; i < nthreads; i++)
	{	
		sprintf(arg0, "%d", i);
		args[0] = arg0;
		pids[i] = mppa_spawn(i, NULL, "km.slave", (const char **)args, NULL);
		assert(pids[i] != -1);
	}
}

/*
 * Joins slave processes.
 */
void join_slaves(void)
{
	int i;
	
	/* Join slaves. */
	for (i = 0; i < nthreads; i++)
	{
		data_receive(infd[i], &slave[i], sizeof(uint64_t));
		mppa_waitpid(pids[i], NULL, 0);
	}
}

/*
 * Waits for slaves to be ready.
 */
void sync_slaves(void)
{
	uint64_t match;

	assert(mppa_read(sync_fd, &match, 8) == 8);
}

/*
 * Open NoC connectors.
 */
void open_noc_connectors(void)
{
	int i;          /* Loop index.     */
	char path[35];  /* Connector path. */
	uint64_t match; /* match value.    */
	
	match = -(1 << nthreads);
	assert((sync_fd = mppa_open("/mppa/sync/128:64", O_RDONLY)) != -1) ;
	assert(mppa_ioctl(sync_fd, MPPA_RX_SET_MATCH, match) != -1);

	/* Open channels. */
	for (i = 0; i < nthreads; i++)
	{		
		sprintf(path, "/mppa/channel/%d:%d/128:%d", i, i + 1, i + 1);
		outfd[i] = mppa_open(path, O_WRONLY);
		assert(outfd[i] != -1);
		
		sprintf(path, "/mppa/channel/128:%d/%d:%d", i + 33, i, i + 33);
		infd[i] = mppa_open(path, O_RDONLY);
		assert(outfd[i] != -1);
	}
}

/*
 * Close NoC connectors.
 */
void close_noc_connectors(void)
{
	int i;
	
	/* Close channels. */
	for (i = 0; i < nthreads; i++)
	{
		mppa_close(outfd[i]);
		mppa_close(infd[i]);
	}
	
	/* Close sync. */
	mppa_close(sync_fd);
}
