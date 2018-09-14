/* Kernel Include */
#include <async_util.h>
#include <message.h>
#include <global.h>
#include <timer.h>
#include <util.h>
#include "matrix.h"
#include "master.h"

/* C And MPPA Library Includes*/
#include <stdint.h>
#include <stdio.h>

/* Arguments sanity check. */
#define SANITY_CHECK()                       \
if (m == NULL) || (l == NULL) || (u == NULL) \
	error("Null Matrix");                    \

/* Message exchange context */
mppa_async_segment_t messages_segment;
mppa_async_segment_t msg_status_segment;
struct message works_inProg[NUM_CLUSTERS];
long long signals[NUM_CLUSTERS] = {0};

/* Timing auxiliars */
static uint64_t start, end;

static void createSegments() {
	createSegment(&messages_segment, MSG_SEG_0, &works_inProg, nclusters * sizeof(struct message), 0, 0, NULL);
}

static void spawnSlaves() {
	start = timer_get();

	/* Parallel spawning PE0 of cluster "i" */
	#pragma omp parallel for default(shared) num_threads(3)
	for (int i = 0; i < nclusters; i++)
		spawn_slave(i, NULL);

	end = timer_get();

	spawn = timer_diff(start, end);
}

static void applyElimination(struct matrix *m) {
	float pivot;         /* Pivot.      */

	for (int i = 0; i < m->height - 1; i++) {
		pivot = find_pivot(m, i, i);	

		/* Impossible to solve. */
		if (pivot == 0.0) {
			warning("cannot factorize matrix");
			break;
		}
		//row_reduction(m, i);
	}
}

/* Performs LU factorization in a matrix */
int matrix_lu(struct matrix *m, struct matrix *l, struct matrix *u) {
	/* Initializes async server */
	async_master_start();

	/* Creates all necessary segments for data exchange */
	createSegments();

	/* SE IGUAL AO ADRESS DE works_inProg ENTAO USAR OS 
	SEG PADROES NOS CC */
	struct message **tester;
	mppa_async_address(&messages_segment, 0, tester);

	/* Spawns all "nclusters" clusters */
	spawnSlaves();

	/* Apply elimination on all rows */
	applyElimination(m);

	/* Waiting for PE0 of each cluster to end */
	join_slaves();

	/* Finalizes async server */
	async_master_finalize();

	return 0;
}