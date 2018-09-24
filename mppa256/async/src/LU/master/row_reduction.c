/* Kernel Include */
#include <async_util.h>
#include <message.h>
#include <global.h>
#include <timer.h>
#include "matrix.h"
#include "master.h"


/* C And MPPA Library Includes*/
#include <stdint.h>
#include <stdio.h>

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

static void receiveResults(int *index, struct message *msg) {
	int i = *index;

	/* Waits new msg. sig. from the last working cluster. */
	mppa_async_evalcond((long long *)&works_inProg[i-1].signal, 1, MPPA_ASYNC_COND_EQ, NULL);
			
	/* Ensures that all msg. put operations are done. */
	mppa_async_fence(&messages_segment, NULL);

	/* Receive results. */
	for (/* NOOP */ ; i > 0; i--) {
		msg = message_get(&messages_segment, i-1, NULL);

	}

	/* Sync i from find_pivot. */
	*index = i;
}

/* Applies the row reduction algorithm in a matrix. */
void row_reduction(struct matrix *m, int i0) {
	int i;               /* Loop indexes.          */
	struct message *msg; /* Message.               */
	
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
		mppa_async_postadd(mppa_async_default_segment(i), sigOffsets[i], 1);

		i++;
		message_destroy(msg);

		if (i == nclusters) 
			receiveResults(&i, msg);
	}

	receiveResults(&i, msg);
}