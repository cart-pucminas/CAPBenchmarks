/* Kernel Includes */
#include <async_util.h>
#include <message.h>
#include <problem.h>
#include <global.h>
#include <timer.h>
#include <util.h>
#include "../common_main.h"

/* C And MPPA Library Includes*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

/* Asynchronous segments */
static mppa_async_segment_t infos_seg;

/* Statistics information from slaves. */
static struct message statistics[NUM_CLUSTERS];

static int *comm_buffer;

/* Timing auxiliars */
static uint64_t start, end;

/* Create segments for data and info exchange. */
static void create_segments() {
	createSegment(&signals_offset_seg, SIG_SEG_0, &sig_offsets, nclusters * sizeof(off64_t), 0, 0, NULL);
	createSegment(&infos_seg, MSG_SEG_0, &statistics, nclusters * sizeof(struct message), 0, 0, NULL);
}

static void spawnSlaves(int nb_towns, int seed) {
	start = timer_get();

	char str_nclusters[10], str_ntowns[10], str_seed[10];
	sprintf(str_nclusters, "%d", nclusters);
	sprintf(str_ntowns, "%d", nb_towns);
	sprintf(str_seed, "%d", seed);

	set_cc_signals_offset();

	/* Parallel spawning PE0 of cluster "i" */
	#pragma omp parallel for default(shared) num_threads(3)
	for (int i = 0; i < nclusters; i++) {
		char *args[5];
		args[0] = str_cc_signals_offset[i];
		args[1] = str_nclusters;
		args[2] = str_ntowns;
		args[3] = str_seed;
		args[4] = NULL;

		spawn_slave(i, args);
	}
	end = timer_get();
	spawn = timer_diff(start, end);
}

void run_tsp (int nb_towns, int seed) {
	assert (nclusters <= NUM_CLUSTERS);

	/* Initializes async server */
	async_master_start();

	int i;
	int nb_partitions = get_number_of_partitions(nclusters);
	int finished_clusters = 0;
	int next_partition = 0;

	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");

	if (verbose)
		printf ("Number of clusters..: %3d\nNumber of partitions: %3d\nNumber of Towns.....: %3d\nSeed................: %3d\n", nclusters , nb_partitions, nb_towns, seed);

	int comm_buffer_size = (nclusters + 1) * sizeof (int);
	comm_buffer = (int *) malloc(comm_buffer_size);
	for (i = 0; i <= nclusters; i++) 
		comm_buffer[i] = INT_MAX;

	/* Creates all necessary segments for data exchange */
	create_segments();

	/* Spawns all clusters getting signal offsets after. */
	spawnSlaves(nb_towns, seed);

	/* Synchronize variable offsets between Master and Slaves. */
	get_slaves_signals_offset();

	/* Waiting for PE0 of each cluster to end */
	join_slaves();

	/* Finalizes async server */
	async_master_finalize();
}