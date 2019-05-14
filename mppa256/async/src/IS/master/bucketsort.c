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

/* Number of buckets. */
#define NUM_BUCKETS 256

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

/* Thread's data. */
static struct tdata {
	int i0;               /* Start bucket.        */
	int in;               /* End bucket.          */
	int j0;               /* Start array's index. */
} tdata[NUM_IO_CORES - 1];

/* Rebuilds array. */
static void rebuild_array(struct bucket **done, int *array) {
	int j;    /* array[] offset. */
	int i, k; /* Loop index.     */
	
	#define BUCKETS_PER_CORE (NUM_BUCKETS/(NUM_IO_CORES-1))

	j = 0;
	/* Setting threads data*/
	for (i = 0; i < NUM_IO_CORES-1; i++) {
		tdata[i].i0 = (i == 0) ? 0 : tdata[i-1].in;
		tdata[i].in = (i == NUM_IO_CORES - 2) ? NUM_BUCKETS : (i + 1) * BUCKETS_PER_CORE;
		tdata[i].j0 = j;

		if (i == NUM_IO_CORES-2) break;

		for (k = tdata[i].i0; k < tdata[i].in; k++)
			j += bucket_size(done[k]);

	}

	/* Rebuild array. */
	#pragma omp parallel for private(i, k, j) default(shared) num_threads(NUM_IO_CORES-1)
	for (i = 0; i < NUM_IO_CORES-1; i++) {
		j = tdata[i].j0;
		for (k = tdata[i].i0; k < tdata[i].in; k++) {
			bucket_merge(done[k], &array[j]);
			j += bucket_size(done[k]);
		}
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