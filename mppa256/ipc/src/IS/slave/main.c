/*
 * Copyright (C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/slave/main.c - Slave main().
 */

#include <arch.h>
#include <assert.h>
#include <global.h>
#include <limits.h>
#include <message.h>
#include <omp.h>
#include <stdlib.h>
#include <stdint.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>
#include <stdio.h>
#include "slave.h"

/* Max size of mini-bucket. */
#define MINIBUCKET_SIZE 262144

/* Tests if partial sorting was successful. */
#define test_partial_order(array, n) {			\
	int l;										\
	for (l = 0; l < ((n)-1); l++) 				\
		assert(((array)[l]) <= ((array)[l+1]));	\
}

/* Tests if there's a negative number in the array */
#define test_non_negatives(array, n) {		\
	int l;									\
	for (l = 0; l < (n); l++) 				\
		assert(((array)[l]) >= 0);			\
}

/* Timing statistics. */
uint64_t start;
uint64_t end;
uint64_t communication = 0;
uint64_t total = 0;

/*  Array block. */
struct  {
	int size;                      /* Size of block. */
	int elements[MINIBUCKET_SIZE]; /* Elements.      */
} block;

/*
 * Sorts an array of numbers.
 */
extern void sort2power(int *array, int size, int chunksize);

static void work() {
	int i, id;           /* Loop index & Bucket ID. */
	int sizeAux;         /* Auxiliary block size.   */
	struct message *msg; /* Message.                */

	/* Slave life. */
	while(1) {
		msg = message_receive(infd);

		if (msg->type == SORTWORK) {
			/* Receive matrix block. */
			block.size = msg->u.sortwork.size;
			data_receive(infd, block.elements, block.size*sizeof(int));
				
			/* Extract message information. */
			id = msg->u.sortwork.id;
			message_destroy(msg);
				
			/* Sorting... */
			start = timer_get();

			sizeAux = block.size;
			while (sizeAux % NUM_THREADS != 0) {
				block.elements[sizeAux] = INT_MAX;
				sizeAux++;
			}

			/* Sorts minibucket. */
			sort2power(block.elements, sizeAux, sizeAux/NUM_THREADS);
			end = timer_get();
			total += timer_diff(start, end);

			// test_partial_order(block.elements, sizeAux);
			// test_non_negatives(block.elements, sizeAux);
				
			/* Send message back.*/
			msg = message_create(SORTRESULT, id, block.size);
			message_send(outfd, msg);
			message_destroy(msg);

			/* Send data sorted. */
			data_send(outfd, block.elements, block.size*sizeof(int));
		} else {
			message_destroy(msg);
			break;		
		}
	}
}

/*
 * Obeys master.
 */
int main(__attribute__((unused))int argc, char **argv) {
	/* Timer synchronization. */
	timer_init();

	/* Util information for the problem. */
	total = 0;
	rank = atoi(argv[0]);

	/* Initializes IPC */
	open_noc_connectors();

	/* Slave life. */
	omp_set_num_threads(NUM_THREADS);
	work();

	/* Put statistics in stats. segment on IO side. */
	data_send(outfd, &total, sizeof(uint64_t));

	/* Finalizes IPC. */
	close_noc_connectors();

	mppa_exit(0);
	return (0);
}
