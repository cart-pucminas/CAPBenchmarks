/* Kernel Includes */
#include <problem.h>
#include <global.h>
#include <timer.h>
#include "../common_main.h"

/* C And MPPA Library Includes*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Timing auxiliars */
static uint64_t start, end;

void run_tsp (int nb_towns, int seed, int nb_clusters) {
	int rank, status = 0, i;
	int pid;
	int nb_partitions = get_number_of_partitions(nb_clusters);
	int finished_clusters = 0;
	int next_partition = 0;

	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");

	if (verbose)
		printf ("Number of clusters..: %3d\nNumber of partitions: %3d\nNumber of Towns.....: %3d\nSeed................: %3d\n", nb_clusters, nb_partitions, nb_towns, seed);
}