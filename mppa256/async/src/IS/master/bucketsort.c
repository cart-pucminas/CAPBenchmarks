/* Kernel Includes */
#include <global.h>
#include <message.h>
#include <timer.h>
#include <util.h>
#include "master.h"

/* C And MPPA Library Includes*/
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

/* Number of buckets. */
#define NUM_BUCKETS 256

/* Debug?. */
#define ISDEBUG 0

/* Tests if partial sorting was successful. */
#define test_partial_order(array, n) {				\
	if (ISDEBUG) {									\
		for (int l = 0; l < ((n)-1); l++) 			\
			assert(((array)[l]) <= ((array)[l+1]));	\
	}												\
}

/* Tests if there's a negative number in the array */
#define test_non_negatives(array, n) {				\
	if (ISDEBUG) {									\
		for (int l = 0; l < (n); l++) 			\
			assert(((array)[l]) >= 0);				\
	}												\
}

/* Data exchange segments. */
static mppa_async_segment_t infos_seg;
static mppa_async_segment_t minibs_seg;

/* Timing auxiliars */
static uint64_t start, end;

/* Msg exchange. */
static struct message statistics[NUM_CLUSTERS];
static int *minibs;

/* Create segments for data and info exchange. */
static void create_segments() {
	createSegment(&signals_offset_seg, SIG_SEG_0, &sig_offsets, nclusters * sizeof(off64_t), 0, 0, NULL);
	createSegment(&infos_seg, MSG_SEG_0, &statistics, nclusters * sizeof(struct message), 0, 0, NULL);
	createSegment(&minibs_seg, 3, minibs, nclusters * MINIBUCKET_SIZE * sizeof(int), 0, 0, NULL);
}

static void spawnSlaves() {
	start = timer_get();

	set_cc_signals_offset();

	/* Parallel spawning PE0 of cluster "i" */
	#pragma omp parallel for default(shared) num_threads(3)
	for (int i = 0; i < nclusters; i++) {
		char *args[2];
		args[0] = str_cc_signals_offset[i];
		args[1] = NULL;

		spawn_slave(i, args);
	}
	end = timer_get();
	spawn = timer_diff(start, end);
}

/* Rebuilds array. */
static void rebuild_array(struct bucket **done, int *array) {
	int j = 0; /* array[] offset. */
	int i, k;  /* Loop index.     */
	
	int first = 1; /* Threads first iteration. */ 
	#pragma omp parallel for private(i, k) firstprivate(j) default(shared) num_threads(3)
	for (i = 0; i < NUM_BUCKETS; i++) {
		// Initializing "j" with the right offset.
		if (first) {
			for (k = 0; k < i; k++)
				j += bucket_size(done[k]);
			first = 0;
		}

		// Merging buckets.
		j += bucket_size(done[i]);
		bucket_merge(done[i], &array[j-1]);
	}
}

static void sort(int *array, int n) {
	int max;                  /* Maximum number.      */
	int i, j;                 /* Loop indexes.        */
	int range;                /* Bucket range.        */
	struct minibucket *minib; /* Working mini-bucket. */
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
			memcpy(&statistics[j], message_create(SORTWORK, i, minib->size), sizeof(struct message));

			/* Send data. */
			memcpy(&minibs[j * MINIBUCKET_SIZE], minib->elements, minib->size * sizeof(int));
			send_signal(j);

			minibucket_destroy(minib);

			/* Next cluster. */
			j++;

			/* All clusters are working, so lets wait for results. */
			if (j == nclusters) {
				/* Receive results. */
				for (j = 0 ; j < nclusters; j++) {
					/* Waiting data to be ready. */
					wait_signal(j);

					/* Receive mini-bucket. */
					minib = minibucket_create();
					minib->size = statistics[j].u.sortwork.size;
					memcpy(minib->elements, &minibs[j * MINIBUCKET_SIZE], minib->size * sizeof(int));

					bucket_push(done[statistics[j].u.sortwork.id], minib);
				}

				j = 0;
			}
		}
	}

	/* Receive remaining results. */
	for (i = 0; i < j; i++) {						
		/* Waiting data to be ready. */
		wait_signal(i);

		/* Receive mini-bucket. */
		minib = minibucket_create();
		minib->size = statistics[i].u.sortwork.size;
		memcpy(minib->elements, &minibs[i * MINIBUCKET_SIZE], minib->size * sizeof(int));

		bucket_push(done[statistics[i].u.sortwork.id], minib);
	}	

	/* Kill slaves. */
	for (i = 0; i < nclusters; i++) {
		memcpy(&statistics[i], message_create(DIE), sizeof(struct message));
		send_signal(i);
	}

	start = timer_get();
	rebuild_array(done, array);
	end = timer_get();
	master += timer_diff(start, end);

	/* House keeping. */
	for (i = 0; i < NUM_BUCKETS; i++) {
		bucket_destroy(todo[i]);
		bucket_destroy(done[i]);
	}
	free(done);
	free(todo);
}

void bucketsort(int *array, int n) {
	minibs = (int *) malloc(nclusters * MINIBUCKET_SIZE * sizeof(int));

	/* Initializes async server */
	async_master_start();

	/* Creates all necessary segments for data exchange */
	create_segments();

	/* Spawns all clusters. */
	spawnSlaves();

	/* Synchronize variable offsets between Master and Slaves. */
	get_slaves_signals_offset();

	/* Begin sort procedures. */
	sort(array, n);

	/* Wait all slaves statistics info. */
	wait_statistics();

	/* Set slaves statistics. */
	set_statistics(statistics);

	/* Waiting for PE0 of each cluster to end */
	join_slaves();

	/* Finalizes async server */
	async_master_finalize();

	/* House keeping. */
	free(minibs);
}