#include <arch.h>
#include <assert.h>
#include <global.h>
#include <math.h>
#include <message.h>
#include <omp.h>
#include <stdlib.h>
#include <stdint.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>
#include "slave.h"

/* Timing statistics. */
uint64_t communication = 0;
uint64_t total = 0;

/* Timing statistics auxiliars. */
static uint64_t start, end;

/* Applies the row reduction algorithm in a matrix. */
void row_reduction(void) {
	int i, j;    /* Loop indexes.   */
	float mult;  /* Row multiplier. */
	float pivot; /* Pivot element.  */
	
	pivot = pvtline.elements[0];
	
	/* Apply row redution in some lines. */
	#pragma omp parallel for private(i, j, mult) default(shared)
	for (i = 0; i < block.height; i++) {
		/* Stores the line multiplier. */
		mult = BLOCK(i, 0)/pivot;

		/* Store multiplier. */
		BLOCK(i, 0) = mult;

		/* Iterate over columns. */
		for (j = 1; j < block.width; j++)
			BLOCK(i, j) = BLOCK(i, j) - mult*pvtline.elements[j];
	}
}

static int doWork() {
	int count; 			 /* Loop indexes.          */
	size_t n;      		 /* Number of bytes to send.  s*/
	int i0, j0;          /* Block start.              */
	struct message *msg; /* Message.                  */

	msg = message_receive(infd);

	switch (msg->type) {
		/* REDUCTRESULT. */
		case REDUCTWORK :
			/* Receive pivot line. */
			n = (msg->u.reductwork.width) * sizeof(float);
			assert(n <= sizeof(pvtline.elements));
			data_receive(infd, pvtline.elements, n);
				
			/* Receive matrix block. */
			for (count = 0; count < msg->u.reductwork.height; count++) {
				n = (msg->u.reductwork.width) * sizeof(float);
				data_receive(infd, &BLOCK(count, 0), n);
			}
			
			/* Extract message information. */
			block.height = msg->u.reductwork.height;
			block.width = pvtline.width = msg->u.reductwork.width;
			i0 = msg->u.reductwork.i0;
			j0 = msg->u.reductwork.j0;
			message_destroy(msg);
			
			start = timer_get();
			row_reduction();
			end = timer_get();
			total += timer_diff(start, end);
				
			/* Send message back.*/
			msg = message_create(REDUCTRESULT, i0, j0, block.height, block.width);
			message_send(outfd, msg);
			message_destroy(msg);
				
			/* Send matrix block. */
			for (count = 0; count < block.height; count++) {
				n = block.width;
				data_send(outfd, &BLOCK(count, 0), n);
			}
				
			/* Slave cant die yet. Needs to wait another message */
			return 1;
				
		/* DIE. */
		default:
			message_destroy(msg);
			return 0;
	}
}

int main(__attribute__((unused))int argc, char **argv) {
	timer_init();
	
	rank = atoi(argv[0]);
	open_noc_connectors();

	/* Slave life. */
	while (doWork());

	/* Sends back time statistics to IO. */
	data_send(outfd, &total, sizeof(uint64_t));

	close_noc_connectors();
	mppa_exit(0);

	return (0);
}
