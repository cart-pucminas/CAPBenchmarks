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

/* Message exchange context */
mppa_async_segment_t messages_segment;
struct message works_inProg[NUM_CLUSTERS];

/* Matrix blocks exchange. */
mppa_async_segment_t matrix_segment;

/* Timing auxiliars */
static uint64_t start, end;

static void createSegments(struct matrix *m) {
	createSegment(&signals_offset_seg, SIG_SEG_0, &sig_offsets, nclusters * sizeof(off64_t), 0, 0, NULL);
	createSegment(&messages_segment, MSG_SEG_0, &works_inProg, nclusters * sizeof(struct message), 0, 0, NULL);
	createSegment(&matrix_segment, MATRIX_SEG_0, &MATRIX(m, 0, 0), (m->height * m->width * sizeof(float)), 0, 0, NULL);
}

static void sync_spawn(int matrix_width) {
	char str_mWidth[10];
	sprintf(str_mWidth, "%d", matrix_width);

	set_cc_signals_offset();

	/* Parallel spawning PE0 of cluster "i" */
	start = timer_get();
	#pragma omp parallel for default(shared) num_threads(3)
	for (int i = 0; i < nclusters; i++) {
		char *args[3];
		args[0] = str_cc_signals_offset[i];
		args[1] = str_mWidth;
		args[2] = NULL;
		spawn_slave(i, args);
	}
	end = timer_get();
	spawn = timer_diff(start, end);

	get_slaves_signals_offset();
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

static void releaseSlaves() {
	struct message *msg;
	for (int i = 0; i < nclusters; i++) {
		msg = message_create(DIE);
		works_inProg[i] = *msg;
		send_signal(i);
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
void matrix_lu(struct matrix *m, struct matrix *l, struct matrix *u) {
	/* Initializes async server */
	async_master_start();

	/* Creates all necessary segments for data exchange */
	createSegments(m);

	/* Spawns all "nclusters" clusters */
	sync_spawn(m->width);

	/* Apply elimination on all rows */
	applyElimination(m);

	/* End all slaves lives. */
	releaseSlaves();

	/* Wait all slaves statistics info. */
	wait_statistics();

	/* Waiting for PE0 of each cluster to end */
	join_slaves();

	/* Finalizes async server */
	async_master_finalize();

	/* Split matrix m into lower and upper matrices. */
	split(m, l, u);
}