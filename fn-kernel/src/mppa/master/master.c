/* 
 * File:   master.c - Mutually Friendly Numbers master process.
 * Authors: 
 *	Matheus A. Souza <matheusalcantarasouza@gmail.com>
 *      Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * Copyright(C) 2014
 *
 */

#include <assert.h>
#include <global.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <mppaipc.h>
#include "master.h"

typedef struct {
    int number;
    int num;
    int den;
} Item;

#define MAX_TASK_SIZE 65536

static Item finishedTasks[MAX_TASK_SIZE*NUM_CLUSTERS];
static Item task[MAX_TASK_SIZE];

/* Parameters.*/
static int end;         /* Start number. */
static int start;       /* End number.   */
static int tasksize;    /* Task size.    */
static int problemsize; /* Problem size. */

/*
 * Sends works to slaves.
 */
static void sendWork(void)
{
    int i, j;               /* Loop indexes.            */
    ssize_t count;          /* Bytes actually sent.     */
    int lowernum, uppernum; /* Lower and upper numbers. */

    /* Distribute tasks to slaves. */
    lowernum = start; uppernum = end;
    for (i = 0; i < nthreads; i++)
    {
		/* Build pool of tasks. */
		for (j = 0; j < tasksize; j += 2)
		{
			task[j].number = lowernum++;
			task[j + 1].number = uppernum--;
		}

        count = mppa_write(outfd[i], task, tasksize*sizeof(Item));
        assert(count != -1);
    }
    
}

static void syncNumbers(int tasksize)
{
	int i;         /* Loop index.          */
    ssize_t count; /* Bytes actually read. */

	for (i = 0; i < nthreads; i++)
	{
        count = mppa_read(infd[i], &finishedTasks[i*tasksize], tasksize*sizeof(Item));
        assert(count != -1);	
	}
}

/*
 * Thread's data.
 */
struct tdata
{
	/* Thread ID. */
	pthread_t tid; 
	 
	struct
	{
		int i0;
		int in;
	} args;
	
	struct
	{
		int nfriends;
	} result;
} tdata[NUM_IO_CORES];

/*
 * Thread's main.
 */
static void *thread_main(void *args)
{
	int i, j;        /* Loop indexes.      */
	int nfriends;    /* Number of friends. */
	struct tdata *t; /* Thread data.       */
	
	t = args;
	
	/* Count number of friends. */
	nfriends = 0;
	for (i = t->args.i0; i < t->args.in; i++)
	{
		for (j = 0; j < i; j++)
		{
			/* Friends. */
			if ((finishedTasks[i].num == finishedTasks[j].num) && 
			(finishedTasks[i].den == finishedTasks[j].den))
				nfriends++;
		}	
	}
	
	t->result.nfriends = nfriends;
	
	pthread_exit(NULL);
	return (NULL);
}

/*
 * Counts friendly numbers.
 */
static int count_friends(void)
{
	int i;        /* Loop index.     */
	int nfriends; /* Number friends. */
	
	/* Spwan slave threads. */
	for (i = 0; i < NUM_IO_CORES; i++)
	{
		tdata[i].args.i0 = (i == 0) ? 1 : i*tasksize;
		tdata[i].args.in = (i + 1)*tasksize;
		pthread_create(&tdata[i].tid, NULL, thread_main, (void *)&tdata[i]);
	}
	
	/* Join threads. */
	for (i = 0; i < NUM_IO_CORES; i++)
		pthread_join(tdata[i].tid, NULL);	
	
	/* Reduce. */
	nfriends = 0;
    for (i = 0; i < NUM_IO_CORES; i++)
		nfriends += tdata[i].result.nfriends;
    
    return (nfriends);
}

/*
 * Counts the number of friendly numbers in a range.
 */
int friendly_numbers(int _start, int _end) 
{
	start = _start;
	end = _end;
	
    /* */
	problemsize = end - start + 1;
	tasksize = problemsize / nthreads;
	
	/* Setup slaves. */
    open_noc_connectors();
	spawn_slaves(tasksize);
    sync_slaves();
    
	sendWork();
    syncNumbers(tasksize);
	
	/* House keeping. */
	join_slaves();
	close_noc_connectors();

    return (count_friends());
}
