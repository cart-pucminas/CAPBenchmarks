/* Kernel Include */
#include <async_util.h>
#include <problem.h>
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
mppa_async_segment_t sigOffsets_segment;
struct message works_inProg[NUM_CLUSTERS];
off64_t sigOffsets[NUM_CLUSTERS] = {0};

/* Matrix blocks exchange. */
mppa_async_segment_t matrix_segment;

/* Timing auxiliars */
static uint64_t start, end;

static void createSegments(struct matrix *m) {
	createSegment(&sigOffsets_segment, SIG_SEG_0, &sigOffsets, nclusters * sizeof(off64_t), 0, 0, NULL);
	createSegment(&messages_segment, MSG_SEG_0, &works_inProg, nclusters * sizeof(struct message), 0, 0, NULL);
	createSegment(&matrix_segment, MATRIX_SEG_0, &MATRIX(m, 0, 0), (m->height * m->width * sizeof(float)), 0, 0, NULL);
}

static void spawnSlaves() {
	/* Information for the slaves about matrix width and height. */
	char str_height[10], str_width[10];
	sprintf(str_height, "%d", prob->height);
	sprintf(str_width, "%d", prob->width);
	char *args[3];
	args[0] = str_height;
	args[1] = str_width;
	args[2] = NULL;

	start = timer_get();

	/* Parallel spawning PE0 of cluster "i" */
	#pragma omp parallel for default(shared) num_threads(3)
	for (int i = 0; i < nclusters; i++)
		spawn_slave(i, args);

	end = timer_get();

	spawn = timer_diff(start, end);
}

static void waitSigOffsets() {
	for (int i = 0; i < nclusters; i++)
		waitCondition(&sigOffsets[i], 0, MPPA_ASYNC_COND_GT, NULL);
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

static void releaseSlaves() {
	for (int i = 0; i < nclusters; i++)
		mppa_async_postadd(mppa_async_default_segment(i), sigOffsets[i], 1);
}

/* Performs LU factorization in a matrix */
int matrix_lu(struct matrix *m, struct matrix *l, struct matrix *u) {
	/* Initializes async server */
	async_master_start();

	/* Creates all necessary segments for data exchange */
	createSegments(m);

	/* Spawns all "nclusters" clusters */
	spawnSlaves();

	/* Wait for all clusters signal offset. */
	waitSigOffsets();

	// Destruir segmento dos offsets

	/* Apply elimination on all rows */
	applyElimination(m);

	/* Releases all slaves that didn't got any tasks. */
	releaseSlaves();

	/* Waiting for PE0 of each cluster to end */
	join_slaves();

	/* Finalizes async server */
	async_master_finalize();

	return 0;
}