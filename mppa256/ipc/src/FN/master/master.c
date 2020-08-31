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
#include <timer.h>
#include <util.h>
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


typedef struct {
    int number;
    int num;
    int den;
} Item;

#define MAX_TASK_SIZE 65536

static Item tasks[MAX_TASK_SIZE];

/* Total of friendly numbers */
static int friendlyNumbers = 0;

/* Parameters.*/
static int startnum;               /* Start number.      */
static int endnum;                 /* End number.        */
static int problemsize;            /* Total tasks size    */
static int avgtasksize;            /* Average tasks size. */
static int tasksize[NUM_CLUSTERS]; /* tasks size.         */

/* Timing statistics. */
static uint64_t start;
static uint64_t end;

static void distributeTaskSizes(int _start, int _end) {
	int i;

	start = timer_get();

	startnum = _start;
	endnum = _end;

	problemsize = (_end - _start + 1);

	if (problemsize > MAX_TASK_SIZE)
		problemsize = MAX_TASK_SIZE;

	avgtasksize = problemsize/nclusters;
	
	/* Distribute tasks sizes. */
	for (i = 0; i < nclusters; i++)
		tasksize[i] = (i + 1 < nclusters)?avgtasksize:problemsize-i*avgtasksize;

	end = timer_get();

	master += timer_diff(start, end);
}

static void setTasks() {
	start = timer_get();

	int i;
	int aux = startnum;

	for (i = 0; i < problemsize; i++)
		tasks[i].number = aux++; 

	end = timer_get();

	master += timer_diff(start, end);
}

/*
 * Send work to slaves.
 */
static void sendWork(void) {
    int offset = 0;

    for (int i = 0; i < nclusters; i++) {
		data_send(outfd[i], &tasksize[i], sizeof(int));
		data_send(outfd[i], &tasks[offset], tasksize[i]*sizeof(Item));
		offset += tasksize[i];
    }
    
}

/*
 * Receives work from slaves.
 */
static void receiveWork(void) {
	int i = 0, offset = 0;

	for (i = 0; i < nclusters; i++) {
        data_receive(infd[i], &tasks[offset], tasksize[i]*sizeof(Item));
        offset += tasksize[i];
	}
}

static void sumFriendlyNumbers() {
	int i, j; /* Loop index. */

	start = timer_get();

	#pragma omp parallel for private(i, j) default(shared) reduction(+: friendlyNumbers)
	for (i = 0; i < problemsize; i++) {
		for (j = i + 1; j < problemsize; j++) {
			if (tasks[i].num == tasks[j].num && tasks[i].den == tasks[j].den)
				friendlyNumbers++;
		}
	}

	end = timer_get();

	master += timer_diff(start, end);
}


/*
 * Counts the number of friendly numbers in a range.
 */
int friendly_numbers(int _start, int _end)  {
	/* Intervals to each cluster */
	distributeTaskSizes(_start, _end);

	/* Initializes tasks number values */
	setTasks();

	/* Setup slaves. */
	start = timer_get();
	open_noc_connectors();
	spawn_slaves();
	end = timer_get();
	spawn = timer_diff(start, end);
    
    /* Send work to slaves. */
	sendWork();

	/* Receive work from slaves. */
	receiveWork();
	
	/* House keeping. */
	join_slaves();
	close_noc_connectors();

	/* Sum all friendly numbers. */
	sumFriendlyNumbers();

	return friendlyNumbers;
}
