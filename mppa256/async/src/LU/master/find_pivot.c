/* Kernel Include */
#include <message.h>
#include <global.h>
#include <timer.h>
#include <util.h>
#include "matrix.h"

/* C And MPPA Library Includes*/
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

float find_pivot(struct matrix *m, int i0, int j0) {
	size_t n;            /* Number of bytes to send. */
	int ipvt;            /* ith index of pivot.      */
	int jpvt;            /* jth index of pivot.      */
	struct message *msg; /* Message.                 */

	start = timer_get();
	works_populate(m, i0, j0);
	end = timer_get();
	master += timer_diff(start, end);

	/* Send work. */
	//while(!empty(works)) {
		/* Pops a message from the worklist */
		pop(works, msg);

		/* Puts a message on message segment. */
		message_put(msg);
	//}
}