/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <assert.h>
#include <global.h>
#include <limits.h>
#include <message.h>
#include <pthread.h>
#include <timer.h>
#include <util.h>
#include <stdio.h>
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


/* Number of buckets. */
#define NUM_BUCKETS 256

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

/* Timing auxiliars */
static uint64_t start, end;

/* Rebuilds array. */
static void rebuild_array(struct bucket **done, int *array) {
	int j = 0; 		/* array[] offset. */
	int i, k;  		/* Loop index.     */
	
	int first = 1;

	#pragma omp parallel private(i, k) firstprivate(first, j) default(shared) num_threads(NUM_IO_CORES)
	{
		#pragma omp for
		for (i = 0; i < NUM_BUCKETS; i++) { 
			if (first) {
				for (k = 0; k < i; k++)
					j += bucket_size(done[k]);
				first = 0;
			}
		}

		#pragma omp barrier

		#pragma omp for
		for (i = 0; i < NUM_BUCKETS; i++) {
			j += bucket_size(done[i]);
			bucket_merge(done[i], &array[j-1]);
		}
	}
}

static void sort(int *array, int n) {
	int max;                  /* Maximum number.      */
	int i, j;                 /* Loop indexes.        */
	int range;                /* Bucket range.        */
	struct minibucket *minib; /* Working mini-bucket. */
	struct message *msg;      /* Working message.     */
	struct bucket **todo;     /* Todo buckets.        */
	struct bucket **done;     /* Done buckets.        */

	todo = smalloc(NUM_BUCKETS*sizeof(struct bucket *));
	done = smalloc(NUM_BUCKETS*sizeof(struct bucket *));

	for (i = 0; i < NUM_BUCKETS; i++) {
		done[i] = bucket_create();
		todo[i] = bucket_create();
	}

	/* Find max number in the array. */
	start = timer_get();
	max = INT_MIN;
	for (i = 0; i < n; i++) {
		/* Found. */
		if (array[i] > max)
			max = array[i];
	}

	/* Distribute numbers. */
	range = max/NUM_BUCKETS;
	for (i = 0; i < n; i++) {
		j = array[i]/range;
		if (j >= NUM_BUCKETS)
			j = NUM_BUCKETS - 1;
		
		bucket_insert(&todo[j], array[i]);
	}
	end = timer_get();
	master += timer_diff(start, end);

	/* Sort buckets. */
	j = 0;
	for (i = 0; i < NUM_BUCKETS; i++) {
		while (bucket_size(todo[i]) > 0) {
			minib = bucket_pop(todo[i]);

			/* Send message. */
			msg = message_create(SORTWORK, i, minib->size);
			message_send(outfd[j], msg);
			message_destroy(msg);

			/* Send data. */
			data_send(outfd[j], minib->elements, minib->size * sizeof(int));
			minibucket_destroy(minib);
			
			/* Next cluster. */
			j++;

			/* All clusters are working, so lets wait for results. */
			if (j == nclusters) {
				/* Receive results. */
				for (j = 0 ; j < nclusters; j++) {
					/* Receive message. */
					msg = message_receive(infd[j]);
					
					/* Receive mini-bucket. */
					minib = minibucket_create();
					minib->size = msg->u.sortresult.size;
					data_receive(infd[j], minib->elements, minib->size * sizeof(int));

					// test_partial_order(minib->elements, minib->size);
					// test_non_negatives(minib->elements, minib->size);
					
					bucket_push(done[msg->u.sortresult.id], minib);
					
					message_destroy(msg);
				}

				j = 0;
			}

		}
	}

	/* Receive remaining results. */
	for (i = 0; i < j; i++) {	
		/* Receive message. */
		msg = message_receive(infd[i]);
					
		/* Receive mini-bucket. */
		minib = minibucket_create();
		minib->size = msg->u.sortresult.size;
		data_receive(infd[i], minib->elements, minib->size * sizeof(int));
					
		bucket_push(done[msg->u.sortresult.id], minib);
					
		message_destroy(msg);
	}

	/* Kill slaves. */
	for (i = 0; i < nclusters; i++) {
		msg = message_create(DIE);
		message_send(outfd[i], msg);
		message_destroy(msg);
	}

	start = timer_get();
	rebuild_array(done, array);
	end = timer_get();
	master += timer_diff(start, end);

	// test_partial_order(array, n);
	// test_non_negatives(array, n);

	/* House keeping. */
	for (i = 0; i < NUM_BUCKETS; i++) {
		bucket_destroy(todo[i]);
		bucket_destroy(done[i]);
	}
	free(done);
	free(todo);
}

/*
 * Bucket-sort algorithm.
 */
extern void bucketsort(int *array, int n) {
	/* Setup IPC. */
	open_noc_connectors();

	/* Spawn slaves. */
	start = timer_get();
	spawn_slaves();
	end = timer_get();
	spawn = timer_diff(start, end);

	/* Begin sort procedures. */
	sort(array, n);

	/* Waiting for PE0 of each cluster to end. */
	join_slaves();

	/* Finalizes IPC. */
	close_noc_connectors();
}
