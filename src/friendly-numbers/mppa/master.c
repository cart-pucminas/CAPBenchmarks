/* 
 * File:   master.c - Mutually Friendly Numbers master process.
 * Authors: 
 *	Matheus A. Souza <matheusalcantarasouza@gmail.com>
 *      Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * Copyright(C) 2014
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <mppaipc.h>
#include <mppa/osconfig.h>
#include "mfn.h"
#include "defines.h"

#define MPPA_FREQUENCY 400
#define NUM_CLUSTERS 16
#define MAX_TASK_SIZE 65536
//#define SHOW_RESULTS 0
//#define START 10
//#define END 1000
//#define NUM_PROCS 16

static int channel_in_fd[NUM_CLUSTERS]; /* Input channels.  */
static int channel_out_fd[NUM_CLUSTERS]; /* Output channels. */
static Item finishedTasks[MAX_TASK_SIZE*NUM_CLUSTERS];
static Item task[MAX_TASK_SIZE];
static uint64_t residual_error = 0;
static int NEW_END;

static inline uint64_t mppa_get_time(void) {
	return __k1_io_read64((void *)0x70084040) / MPPA_FREQUENCY;
}

static inline uint64_t mppa_diff_time(uint64_t t1, uint64_t t2) {
	return t2 - t1 - residual_error;
}

static void mppa_init_time(void) {
	uint64_t t1, t2;
	t1 = mppa_get_time();
	t2 = mppa_get_time();
	residual_error = t2 - t1;
}

static void sendWork(long int TASK_SIZE, long int middle) {
    int i, j, cont;
    long int lowerNumber, upperNumber;
	ssize_t n;
    ssize_t count;    

    // Distribute numbers in a "balanced way".
    for (i = 0; i < NUM_PROCS; i++) {
		cont = 0;
        lowerNumber = (i * TASK_SIZE / 2) + START;
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

        count = mppa_write(channel_out_fd[i], &n, sizeof(ssize_t));
        assert(count != -1);
        count = mppa_write(channel_out_fd[i], &task, n);
        assert(count != -1);
    }
}

static void syncNumbers(long int TASK_SIZE) {
    ssize_t n, nOfN;
    ssize_t count;
	int i,j;

    nOfN = sizeof (long int);

    for (i = 0; i < NUM_PROCS; i++) {
        count = mppa_read(channel_in_fd[i], &n, nOfN);
        assert(count != -1);
        count = mppa_read(channel_in_fd[i], &task, n);
        assert(count != -1);

		for(j=0;j<TASK_SIZE; j++) 
			finishedTasks[TASK_SIZE*i+j]=task[j];
    }
}

static void showResult(long int TASK_SIZE) {
	int i,j;
    for (i = 0; i < TASK_SIZE*NUM_PROCS; i++) {
        for (j = i + 1; j < TASK_SIZE*NUM_PROCS; j++) {
            if ((finishedTasks[i].numerator == finishedTasks[j].numerator) && (finishedTasks[i].denominator == finishedTasks[j].denominator)) {
				printf("%ld and %ld are FRIENDLY\n", finishedTasks[i].number, finishedTasks[j].number);
            }
        }
    }
}

static void initTasks(long int *PROBLEM_SIZE, long int *TASK_SIZE, long int *middle) {
	long int r, pSizeCheck;
	
    *PROBLEM_SIZE = END - START + 1; // "Problem size"
	
    if((*PROBLEM_SIZE) > (MAX_TASK_SIZE*NUM_PROCS)) {
		printf("The Problem Size is too big... The maximum size is %d using %d Clusters.\n", MAX_TASK_SIZE, NUM_CLUSTERS);
		exit(0);
	}
	
	if((NUM_PROCS < 1) || (NUM_PROCS > 16)) {
		printf("The Number of Processors must be a power of 2, between 1 and 16...\n");
		exit(0);
	} else if (!((NUM_PROCS != 0) && !(NUM_PROCS & (NUM_PROCS - 1)))) {
		printf("The Number of Processors must be a power of 2, between 1 and 16...\n");
		exit(0);
	}
	
	if(NUM_PROCS > 8) {
		pSizeCheck = 32;
	} else {
		pSizeCheck = 16;
	}
	r = *PROBLEM_SIZE % pSizeCheck;
	
    if (r > 0) {
        NEW_END = END + (pSizeCheck - r);
		*PROBLEM_SIZE = NEW_END - START + 1;
        printf("The problem size must be multiple of %ld. Using %d to %d.\n",pSizeCheck, START, NEW_END);
		printf("New problem size: %ld\n\n",*PROBLEM_SIZE);
    }
	*middle = *PROBLEM_SIZE / 2 + START;	
	
    // Calculate the TASK_SIZE, based on NUM_PROCS and PROBLEM_SIZE
    *TASK_SIZE = *PROBLEM_SIZE / NUM_PROCS;
	
	printf("Number of Clusters: %d\n",NUM_PROCS);
	printf("Task size / Cluster: %ld\n\n",*TASK_SIZE);
}

int main(void) {
    int sync_fd, i;
	long int middle, PROBLEM_SIZE, TASK_SIZE;
	uint64_t match, start_time, exec_time;
    char path[35];
    char arg0[4];
	char arg1[6];
    char *args[3];
	
	initTasks(&PROBLEM_SIZE, &TASK_SIZE, &middle);

    // Get initial time
	mppa_init_time();
	start_time = mppa_get_time();

    pid_t pids[NUM_PROCS]; // Processes IDs.
    match = -(1 << NUM_PROCS);
    sync_fd = mppa_open("/mppa/sync/128:64", O_RDONLY);
    assert(sync_fd != -1);
    assert(mppa_ioctl(sync_fd, MPPA_RX_SET_MATCH, match) != -1);

    // Open channels.
    for (i = 0; i < NUM_PROCS; i++) {
        sprintf(path, "/mppa/channel/%d:%d/128:%d", i, i + 1, i + 1);
        channel_out_fd[i] = mppa_open(path, O_WRONLY);
        assert(channel_out_fd[i] != -1);
        sprintf(path, "/mppa/channel/128:%d/%d:%d", i + 33, i, i + 33);
        channel_in_fd[i] = mppa_open(path, O_RDONLY);
        assert(channel_in_fd[i] != -1);
    }

    // Spawn slaves.
    args[2] = NULL;
    for (i = 0; i < NUM_PROCS; i++) {
		sprintf(arg0, "%d", i);
        args[0] = arg0;
		sprintf(arg1, "%ld", TASK_SIZE);
        args[1] = arg1;
        pids[i] = mppa_spawn(i, NULL, "slave", (const char **) args, NULL);
        assert(pids[i] != -1);
    }

    // Wait for slaves to be ready.
    assert(mppa_read(sync_fd, &match, 8) == 8);
    mppa_close(sync_fd);
	
	sendWork(TASK_SIZE, middle);
    syncNumbers(TASK_SIZE);

    // Get final time
    exec_time = mppa_diff_time(start_time, mppa_get_time());
	printf ("Execution Time: %.4f\n", exec_time/1000000.0);

    // Close channels.
    for (i = 0; i < NUM_PROCS; i++) {
        mppa_close(channel_out_fd[i]);
        mppa_close(channel_in_fd[i]);
    }

    //Join slaves.
    for (i = 0; i < NUM_PROCS; i++) {
        mppa_waitpid(pids[i], NULL, 0);
    }
	if(SHOW_RESULTS!=0) {
		printf("\nShowing results...\n");
		showResult(TASK_SIZE);
	}
    return (0);
}
