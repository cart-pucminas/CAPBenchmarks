/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/master/row_reduction.c - row_reduction() implementation.
 */
 
#include <arch.h>
#include <global.h>
#include <math.h>
#include <message.h>
#include <stdint.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>
#include <stdio.h>
#include "master.h"
#include "matrix.h"

/* Work list. */
static struct message *works = NULL; 

/* Timing statistics. */
uint64_t end;
uint64_t start;

static void works_populate(struct matrix *m, int i0, int j0) {
	int i;               /* Loop index.     */
	int height;          /* Number of rows. */
	struct message *msg; /* Work.           */
	
	height = (CLUSTER_WORKLOAD/sizeof(float))/((m->width - j0)*sizeof(float));
	
	/* Populate works. */
	for (i = i0 + 1; i < m->height; i += height) {
		if (i + height > m->height)
			height = m->height - i;
		
		msg = message_create(REDUCTWORK, i0, i, j0, height, m->width - j0);
		push(works, msg);
	}
}

static void waitResults(int *index, struct matrix *m) {
	struct message *msg;
	int i = *index; 
	int count;    
	size_t n;           

	/* Waits reduct work from all working clusters. */
	for (/* NOOP */ ; i > 0; i--) {
		msg = message_receive(infd[i - 1]);
				
		/* Receive matrix block. */
		n = (msg->u.reductresult.width) * sizeof(float);

		for (count = 0; count < msg->u.reductresult.height; count++) {
			data_receive(infd[i-1], &MATRIX(m, msg->u.reductresult.i0 + count, msg->u.reductresult.j0), n);
		}
				
		message_destroy(msg);
	}

	*index = i;
}

/* Applies the row reduction algorithm in a matrix. */
void row_reduction(struct matrix *m, int i0) {
	int i, count; 			/* Loop indexes.          */
	size_t n;            	/* Bytes to send/receive. */
	struct message *msg; 	/* Message.               */
	
	start = timer_get();
	works_populate(m, i0, i0);
	end = timer_get();
	master += timer_diff(start, end);
	
	/* Send work. */
	i = 0;
	while (!empty(works)) {	
		/* Pops a message from the worklist. */
		pop(works, msg);
		
		/* Send message. */
		message_send(outfd[i], msg);
		
		/* Send pivot line. */
		n = (msg->u.reductwork.width) * sizeof(float);
		data_send(outfd[i], &MATRIX(m, msg->u.reductwork.ipvt, msg->u.reductwork.j0), n);
		
		/* Send matrix block. */
		for (count = 0; count < msg->u.reductwork.height; count++) {
			n = (msg->u.reductwork.width) * sizeof(float);
			data_send(outfd[i], &MATRIX(m, msg->u.reductwork.i0 + count, msg->u.reductwork.j0), n);
		}
		
		i++;
		message_destroy(msg);
		
		/* All slaves are working. Waiting for their results. */
		if (i == nclusters)
			waitResults(&i, m);
	}
	
	if (i > 0)
		waitResults(&i, m);
}
