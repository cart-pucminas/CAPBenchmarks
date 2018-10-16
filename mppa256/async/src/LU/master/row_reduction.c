/* Kernel Include */
#include <async_util.h>
#include <message.h>
#include <global.h>
#include <problem.h>
#include <timer.h>
#include "matrix.h"
#include "master.h"


/* C And MPPA Library Includes*/
#include <stdint.h>
#include <stdio.h>
#include <math.h>

/* Work list. */
static struct message *works = NULL; 

/* Timing auxiliars. */
uint64_t end;
uint64_t start;

static void works_populate(struct matrix *m, int i0, int j0) {
	int i;               /* Loop index.     */
	int height;          /* Number of rows. */
	struct message *msg; /* Work.           */
	
	height = (CLUSTER_WORKLOAD/sizeof(float))/((m->width - j0)*sizeof(float));
	
	/* Populate works. */
	for (i = i0 + 1; i < m->height; i += height)
	{
		if (i + height > m->height)
			height = m->height - i;
		
		msg = message_create(REDUCTWORK, i0, i, j0, height, m->width - j0);
		push(works, msg);
	}
}

static void waitResults(int *index) {
	int i = *index;

	/* Waits reduct work done signal from all working clusters. */
	for (/* NOOP */ ; i > 0; i--) {
		waitCondition(&cluster_signals[i-1], 1, MPPA_ASYNC_COND_EQ, NULL);	
		
		/* Reset signal for the next iteration. */
		cluster_signals[i-1] = 0;
	}
	
	/* Ensures that all block put operations are done. */
	waitAllOpCompletion(&matrix_segment, NULL);

	*index = i;
}

/* Applies the row reduction algorithm in a matrix. */
void row_reduction(struct matrix *m, int i0) {
	int i;               /* Loop indexes. */
	struct message *msg; /* Message.      */
	
	start = timer_get();
	works_populate(m, i0, i0);
	end = timer_get();
	master += timer_diff(start, end);

	/* Send work. */
	i = 0;
	while(!empty(works)) {
		/* Pops a message from the worklist. */
		pop(works, msg);

		/* Puts a message on the msg segment. */
		works_inProg[i] = *msg;

		/* Sends "message ready" signal to cluster "i". */
		postAdd(mppa_async_default_segment(i), sigOffsets[i], 1);

		i++;
		message_destroy(msg);

		/* All slaves are working. Waiting for their results. */
		if (i == nclusters)
			waitResults(&i);
	}

	if (i > 0)
		waitResults(&i);
}