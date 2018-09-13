/* Kernel Include */
#include <async_util.h>
#include <global.h>
#include <timer.h>
#include <util.h>
#include "matrix.h"

/* C And MPPA Library Includes*/
#include <stdint.h>

/* Arguments sanity check. */
#define SANITY_CHECK()                       \
if (m == NULL) || (l == NULL) || (u == NULL) \
	error("Null Matrix");                    \

/* Timing auxiliars */
static uint64_t start, end;

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