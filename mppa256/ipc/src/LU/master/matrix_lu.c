/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/matrix_lu/master.c - matrix_lu() master.
 */

#include <arch.h>
#include <global.h>
#include <message.h>
#include <stdint.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>
#include "matrix.h"
#include "master.h"

/* Applies the row reduction algorithm in a matrix. */
extern void row_reduction(struct matrix *m, int i0);

/* Timing auxiliars */
static uint64_t start, end;

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
	int i; /* Loop index. */
	struct message *msg;

	for (i = 0; i < nclusters; i++) {
		msg = message_create(DIE);
		message_send(outfd[i], msg);
		message_destroy(msg);
	}
}

static void split(struct matrix *m, struct matrix *l, struct matrix *u) {
	int i, j; /* Loop indexes. */

	start = timer_get();
	#pragma omp parallel for default(shared)
	for (i = 0; i < m->height; i++) {
		for (j = 0; j < m->width; j++) {
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
	int i;               /* Loop index. */
	float pivot;         /* Pivot.      */
	uint64_t start, end; /* Timer.      */
	
	/* Setup slaves. */
	open_noc_connectors();

	start = timer_get();
	spawn_slaves();
	end = timer_get();
	spawn = timer_diff(start, end);

	/* Apply elimination on all rows */
	applyElimination(m);

	/* End all slaves lives. */
	releaseSlaves();
		
	/* Waiting for PE0 of each cluster to end */
	join_slaves();
	close_noc_connectors();

	/* Split matrix m into lower and upper matrices. */
	split(m, l, u);
}
