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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <mppaipc.h>
#include "master.h"

typedef struct {
    long int number;
    long int numerator;
    long int denominator;
} Item;

#define MPPA_FREQUENCY 400
#define NUM_CLUSTERS 16
#define MAX_TASK_SIZE 65536

static Item finishedTasks[MAX_TASK_SIZE*NUM_CLUSTERS];
static Item task[MAX_TASK_SIZE];
static int NEW_END;

int start, end;

static void sendWork(long int TASK_SIZE, long int middle) {
    int i, j, cont;
    long int lowerNumber, upperNumber;
	ssize_t n;
    ssize_t count;    

    // Distribute numbers in a "balanced way".
    for (i = 0; i < nthreads; i++) {
		cont = 0;
        lowerNumber = (i * TASK_SIZE / 2) + start;
        for (j = 0; (j < TASK_SIZE / 2) && (lowerNumber < middle); j++) {
            Item item;
            item.number = lowerNumber;
            task[cont] = item;
            lowerNumber++;
            cont++;
        }

        upperNumber = NEW_END - (i * TASK_SIZE / 2);
        for (j = 0; (j < TASK_SIZE / 2) && (upperNumber >= middle); j++) {
            Item item;
            item.number = upperNumber;
            task[cont] = item;
            upperNumber--;
            cont++;
        }

		n = TASK_SIZE * sizeof (Item);

        count = mppa_write(outfd[i], &n, sizeof(ssize_t));
        assert(count != -1);
        count = mppa_write(outfd[i], &task, n);
        assert(count != -1);
    }
}

static void syncNumbers(long int TASK_SIZE) {
    ssize_t n, nOfN;
    ssize_t count;
	int i,j;

    nOfN = sizeof (long int);

    for (i = 0; i < nthreads; i++) {
        count = mppa_read(infd[i], &n, nOfN);
        assert(count != -1);
        count = mppa_read(infd[i], &task, n);
        assert(count != -1);

		for(j=0;j<TASK_SIZE; j++) 
			finishedTasks[TASK_SIZE*i+j]=task[j];
    }
}

static int showResult(long int TASK_SIZE) {
	int i,j, nfriends;
	
	nfriends = 0;
    for (i = 0; i < TASK_SIZE*nthreads; i++) {
        for (j = i + 1; j < TASK_SIZE*nthreads; j++) {
            if ((finishedTasks[i].numerator == finishedTasks[j].numerator) && (finishedTasks[i].denominator == finishedTasks[j].denominator))
				nfriends++;
        }
    }
    
    return (nfriends);
}

static void initTasks(long int *PROBLEM_SIZE, long int *TASK_SIZE, long int *middle) {
	long int r, pSizeCheck;
	
    *PROBLEM_SIZE = end - start + 1; // "Problem size"
	
    if((*PROBLEM_SIZE) > (MAX_TASK_SIZE*nthreads)) {
		printf("The Problem Size is too big... The maximum size is %d using %d Clusters.\n", MAX_TASK_SIZE, NUM_CLUSTERS);
		exit(0);
	}
	
	if(nthreads > 8) {
		pSizeCheck = 32;
	} else {
		pSizeCheck = 16;
	}
	r = *PROBLEM_SIZE % pSizeCheck;
	
    if (r > 0) {
        NEW_END = end + (pSizeCheck - r);
		*PROBLEM_SIZE = NEW_END - start + 1;
        printf("The problem size must be multiple of %ld. Using %d to %d.\n",pSizeCheck, start, NEW_END);
		printf("New problem size: %ld\n\n",*PROBLEM_SIZE);
    }
	*middle = *PROBLEM_SIZE / 2 + start;	
	
    // Calculate the TASK_SIZE, based on nthreads and PROBLEM_SIZE
    *TASK_SIZE = *PROBLEM_SIZE / nthreads;
}

int friendly_numbers(int _start, int _end) 
{
    int nfriends;
	long int middle, PROBLEM_SIZE, TASK_SIZE;
	
	start = _start;
	end = _end;
	
	initTasks(&PROBLEM_SIZE, &TASK_SIZE, &middle);

    open_noc_connectors();

	spawn_slaves(TASK_SIZE);

    sync_slaves();
	
	sendWork(TASK_SIZE, middle);
    syncNumbers(TASK_SIZE);
	
	/* House keeping. */
	join_slaves();
	close_noc_connectors();

	nfriends = showResult(TASK_SIZE);

    return (nfriends);
}
