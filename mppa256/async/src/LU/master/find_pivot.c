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

/* Swaps two columns in a matrix */
static void _swap_columns(struct matrix *m, int j1, int j2) {
	float tmp; /* Temporary value. */

	/* Swap columns. */
	for (int i = 0; i < m->height; i++) {
		tmp = MATRIX(m, i, j1);
		MATRIX(m, i, j1) = MATRIX(m, i, j2);
		MATRIX(m, i, j2) = tmp;
	}
}

/* Populates work list. */
static void works_populate(struct matrix *m, int i0, int j0) {
	int height;          /* Number of rows. */
	struct message *msg; /* Work.           */
	
	height = (CLUSTER_WORKLOAD/sizeof(float))/((m->width - j0)*sizeof(float));
	
	/* Populate works. */
	for (int i = i0; i < m->height; i += height) {
		if (i + height > m->height)
			height = m->height - i;
		
		msg = message_create(FINDWORK, i, j0, height, m->width - j0);
		
		push(works, msg);
	}
}

static void receiveResults(int *index, struct message *msg) {
	int i = *index;

	/* Waits new msg. sig. from the last working cluster. */
	mppa_async_evalcond((long long *)&works_inProg[i-1].signal, 1, MPPA_ASYNC_COND_EQ, NULL);
			
	/* Ensures that all msg. put operations are done. */
	mppa_async_fence(&messages_segment, NULL);

	/* Receive results. */
	for (/* NOOP */ ; i > 0; i--) {
		msg = message_get(&messages_segment, i-1, NULL);
		push(results,msg);
	}

	/* Sync i from find_pivot. */
	*index = i;
}

float find_pivot(struct matrix *m, int i0, int j0) {
	int i,j;             /* Loop index.              */
	size_t n;            /* Number of bytes to send. */
	int ipvt;            /* ith index of pivot.      */
	int jpvt;            /* jth index of pivot.      */
	struct message *msg; /* Message.                 */

	start = timer_get();
	works_populate(m, i0, j0);
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

	receiveResults(&i, msg);

	start = timer_get();

	/* Find pivot. */
	ipvt = i0;
	jpvt = j0;
	while (!empty(results))
	{
		pop(results, msg);
		
		i = msg->u.findresult.ipvt + msg->u.findresult.i0;
		j = msg->u.findresult.jpvt;
		
		if (fabs(MATRIX(m, i, j)) > fabs(MATRIX(m, ipvt, jpvt)))
		{
			ipvt = i;
			jpvt = j;
		}
		
		message_destroy(msg);
	}

	_swap_rows(m, i0, ipvt);
	_swap_columns(m, j0, jpvt);
	
	
	end = timer_get();
	master += timer_diff(start, end);

	return 0;
	//return (MATRIX(m, ipvt, jpvt));
}