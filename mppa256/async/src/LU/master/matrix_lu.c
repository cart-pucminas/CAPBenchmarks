/* Kernel Include */
#include <async_util.h>
#include <message.h>
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
#include <pthread.h>

/* Message exchange context */
mppa_async_segment_t messages_segment;
mppa_async_segment_t sigOffsets_segment;
struct message works_inProg[NUM_CLUSTERS];
off64_t sigOffsets[NUM_CLUSTERS] = {0};
long long cluster_signals[NUM_CLUSTERS] = {0};

/* Matrix blocks exchange. */
mppa_async_segment_t matrix_segment;

/* Timing auxiliars */
static uint64_t start, end;

static void createSegments(struct matrix *m) {
	createSegment(&sigOffsets_segment, SIG_SEG_0, &sigOffsets, nclusters * sizeof(off64_t), 0, 0, NULL);
	createSegment(&messages_segment, MSG_SEG_0, &works_inProg, nclusters * sizeof(struct message), 0, 0, NULL);
	createSegment(&matrix_segment, MATRIX_SEG_0, &MATRIX(m, 0, 0), (m->height * m->width * sizeof(element)), 0, 0, NULL);
	createSegment(&infos_segment, INFOS_SEG_0, &infos, nclusters * sizeof(Info), 0, 0, NULL);
}

static void spawnSlaves(int matrix_width) {
	start = timer_get();

	char str_mWidth[10];
	sprintf(str_mWidth, "%d", matrix_width);

	/* Parallel spawning PE0 of cluster "i" */
	#pragma omp parallel for default(shared) num_threads(3)
	for (int i = 0; i < nclusters; i++) {
		off64_t offset;
		mppa_async_offset(MPPA_ASYNC_DDR_0, &cluster_signals[i], &offset);
		char str_sig_offset[50];
		sprintf(str_sig_offset, "%lld", offset);
		char *args[3];
		args[0] = str_sig_offset;
		args[1] = str_mWidth;
		args[2] = NULL;
		spawn_slave(i, args);
	}

	end = timer_get();

	spawn = timer_diff(start, end);
}

static void waitSigOffsets() {
	for (int i = 0; i < nclusters; i++)
		waitCondition(&sigOffsets[i], 0, MPPA_ASYNC_COND_GT, NULL);
}

static void applyElimination(struct matrix *m) {
	float pivot;         /* Pivot.      */
	int i;
	for (i = 0; i < m->height - 1; i++) {

		pivot = MATRIX(m, i, i);

		/* Impossible to solve. */
		if (pivot == 0.0) {
			warning("cannot factorize matrix into LU");
			break;
		}

		row_reduction(m, i);
	}
}

static void waitStatistics() {
	for (int i = 0; i < nclusters; i++) 
		mppa_async_evalcond(&cluster_signals[i], 1, MPPA_ASYNC_COND_EQ, NULL);
}

static void releaseSlaves() {
	struct message *msg;
	for (int i = 0; i < nclusters; i++) {
		msg = message_create(DIE);
		works_inProg[i] = *msg;
		mppa_async_postadd(mppa_async_default_segment(i), sigOffsets[i], 1);
		message_destroy(msg);
	}
}


static void split(struct matrix *m, struct matrix *l, struct matrix *u) {
	start = timer_get();

	#pragma omp parallel for default(shared) num_threads(3)
	for (int i = 0; i < m->height; i++) {
		for (int j = 0; j < m->width; j++) {
			if (j > i) {
				MATRIX(u, i, j) = MATRIX(m, i, j);
				MATRIX(l, i, j) = 0.0;
			} else if (j < i) {
				MATRIX(u, i, j) = 0.0;
				MATRIX(l, i, j) = MATRIX(m, i, j);
			} else {
				MATRIX(l,i,j) = 1.0;
				MATRIX(u,i,j) = MATRIX(m,i,j);
			}
		}
	}
	end = timer_get();
	master += timer_diff(start, end);
}

/* Performs LU factorization in a matrix */
int matrix_lu(struct matrix *m, struct matrix *l, struct matrix *u) {
	/* Initializes async server */
	async_master_start();

	/* Creates all necessary segments for data exchange */
	createSegments(m);

	/* Spawns all "nclusters" clusters */
	spawnSlaves(m->width);

	/* Wait for all clusters signal offset. */
	waitSigOffsets();

	/* Destroy signal offsets segment. */
	mppa_async_segment_destroy(&sigOffsets_segment);

	/* Apply elimination on all rows */
	applyElimination(m);

	/* End all slaves lives. */
	releaseSlaves();

	/* Wait all slaves statistics info. */
	waitStatistics();

	/* Waiting for PE0 of each cluster to end */
	join_slaves();

	/* Finalizes async server */
	async_master_finalize();

	/* Split matrix m into lower and upper matrices. */
	split(m, l, u);
	
	return 0;
}