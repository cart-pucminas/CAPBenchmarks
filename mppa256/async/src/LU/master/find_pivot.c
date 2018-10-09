/* Kernel Include */
#include <async_util.h>
#include <message.h>
#include <problem.h>
#include <global.h>
#include <timer.h>
#include <util.h>
#include "matrix.h"
#include "master.h"

/* C And MPPA Library Includes*/
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Lists. */
static struct message *works = NULL;   /* List with all works. */
static struct message *results = NULL; /* Result list.         */

/* Timing auxiliars. */
uint64_t end;
uint64_t start;

/* Swaps two rows of a matrix. */
static void _swap_rows(struct matrix *m, int i1, int i2) {
	float tmp; /* Temporary value. */
	
	/* Swap columns. */
	for (int j = 0; j < m->width; j++) {
		tmp = MATRIX(m, i1, j);
		MATRIX(m, i1, j) = MATRIX(m, i2, j);
		MATRIX(m, i2, j) = tmp;
	}
}

/* Populates work list. */
static void works_populate(struct matrix *m, int i0) {
	int height;          /* Number of rows.   */
	struct message *msg; /* Work.             */

	height = (m->height-i0)/nclusters;
	if ((height*sizeof(float)) > (CLUSTER_WORKLOAD/sizeof(float))) {
		height = (CLUSTER_WORKLOAD/sizeof(float))/((m->height - i0)*sizeof(float));
		
		/* Populate works based on works size. */
		for (int i = i0; i < m->height; i += height) {
			if (i + height > m->height)
				height = m->height - i;
		
			msg = message_create(FINDWORK, i, height);
		
			push(works, msg);
		}
	} else {
		/* populate works in order to use all clusters. */
		int rowCounter = i0;
		for (int i = 0; i < nclusters; i++) {
			if (i == (nclusters-1))
				height = m->height - rowCounter;

			msg = message_create(FINDWORK, height, height);
		
			push(works, msg);

			rowCounter += height;
		}
	}
}

/* Receive work results from clusters. */
static void receiveResults(int *index, struct message *msg) {
	int i = *index;

	for (/* NOOP */ ; i > 0; i--) {
		msg = message_create(DIE);

		/* Waits from msg signal from "i"th cluster. */
		mppa_async_evalcond(&cluster_signals[i-1], 1, MPPA_ASYNC_COND_EQ, NULL);
		
		/* Reset signal for the next iteration. */
		cluster_signals[i-1] = 0;

		*msg = works_inProg[i-1];

		push(results,msg);
	}

	/* Sync i from find_pivot. */
	*index = i;
}

float find_pivot(struct matrix *m, int i0) {
	int i;             /* Loop index.              */
	int ipvt;            /* ith index of pivot.      */
	struct message *msg; /* Message.                 */

	start = timer_get();
	works_populate(m, i0);
	end = timer_get();
	master += timer_diff(start, end);

	i = 0;
	/* Send work. */
	while(!empty(works)) {
		/* Pops a message from the worklist. */
		pop(works, msg);

		/* Puts a message on the msg segment. */
		works_inProg[i] = *msg;

		/* Sends "message ready" signal to cluster "i". */
		mppa_async_postadd(mppa_async_default_segment(i), sigOffsets[i], 1);

		i++;
		message_destroy(msg);

		/* All slaves working. Waiting for results. */
		if (i == nclusters) 
			receiveResults(&i, msg);
	}

	if (i > 0) 
		receiveResults(&i, msg);

	start = timer_get();

	/* Find pivot. */
	ipvt = i0;
	while (!empty(results)) {
		pop(results, msg);
		
		i = msg->u.findresult.ipvt;
		
		if (fabs(MATRIX(m, i, i)) > fabs(MATRIX(m, ipvt, ipvt)))
			ipvt = i;

		message_destroy(msg);
	}

	_swap_rows(m, i0, ipvt);
	
	end = timer_get();
	master += timer_diff(start, end);

	return (MATRIX(m, i0, i0));
}