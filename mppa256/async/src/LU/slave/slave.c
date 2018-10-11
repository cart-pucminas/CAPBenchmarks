/* Kernel Include */
#include <async_util.h>
#include <message.h>
#include <global.h>
#include <timer.h>
#include <util.h>
#include "slave.h"

/* C And MPPA Library Includes*/
#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Data exchange segments. */
static mppa_async_segment_t matrix_segment;
static mppa_async_segment_t messages_segment;
static mppa_async_segment_t sigOffsets_segment;

/* Information to send back to IO */
static mppa_async_segment_t infos_segment;
static Info info = {0, 0, 0, 0, 0, 0};

/* Data exchange signals between IO and Clusters. */
static long long io_signal;
static off64_t sigback_offset;

/* Matrix width for matrix dataPut and dataGet. */
static int matrix_width;

/* Individual Slave statistics. */
uint64_t total = 0;          /* Time spent on slave.    */
uint64_t communication = 0;  /* Time spent on comms.    */               
size_t data_put = 0;         /* Number of bytes put.    */
size_t data_get = 0;         /* Number of bytes gotten. */
unsigned nput = 0;           /* Number of put ops.      */
unsigned nget = 0;           /* Number of get ops.      */

/* Timing statistics auxiliars. */
static uint64_t start, end;

/* Compute Cluster ID */
int cid;

static void sendStatisticsToIO() {
	info.data_put = data_put;
	info.data_get = data_get;
	info.nput = nput;
	info.nget = nget;
	info.slave = total;
	info.communication = communication;

	dataPut(&info, &infos_segment, cid, 1, sizeof(Info), NULL);

	/* Send stats. ready signal to IO. */
	postAdd(MPPA_ASYNC_DDR_0, sigback_offset, 1);
}

static void cloneSegments() {
	cloneSegment(&sigOffsets_segment, SIG_SEG_0, 0, 0, NULL);
	cloneSegment(&messages_segment, MSG_SEG_0, 0, 0, NULL);
	cloneSegment(&matrix_segment, MATRIX_SEG_0, 0, 0, NULL);
	cloneSegment(&infos_segment, INFOS_SEG_0, 0, 0, NULL);
}

static void sendSigOffset() {
	off64_t offset = 0;
	mppa_async_offset(mppa_async_default_segment(cid), &io_signal, &offset);
	dataPut(&offset, &sigOffsets_segment, cid, 1, sizeof(off64_t), NULL);
}

/* Applies the row reduction algorithm in a matrix. */
void row_reduction(void) {
	float mult;  /* Row multiplier. */
	float pivot; /* Pivot element.  */
	
	pivot = pvtline.elements[0];
	
	/* Apply row redution in some lines. */
	#pragma omp parallel for private(mult) default(shared)
	for (int i = 0; i < block.height; i++) {
		/* Stores the line multiplier. */
		mult = BLOCK(i, 0)/pivot;

		/* Store multiplier. */
		BLOCK(i, 0) = mult;

		/* Iterate over columns. */
		for (int j = 1; j < block.width; j++)
			BLOCK(i, j) = BLOCK(i, j) - mult*pvtline.elements[j];
	}
}
int resp = 0;

static int doWork() {
	int i0, j0;          /* Block start.              */
	struct message *msg; /* Message.                  */

	/* Waits for message io_signal to continue. */
	waitCondition(&io_signal, 1, MPPA_ASYNC_COND_EQ, NULL);

	/* Resets io_signal for the next message. */
	io_signal = 0;

	/* Get message from messages remote segment. */
	msg = message_get(&messages_segment, cid, NULL);
	
	switch (msg->type)	{
		/* REDUCTRESULT. */
		case REDUCTWORK :
		/* Receive pivot line. */
		dataGet(&pvtline.elements, &matrix_segment, OFFSET(matrix_width, msg->u.reductwork.ipvt, msg->u.reductwork.j0), msg->u.reductwork.width, sizeof(float), NULL);

		/* Extract message information. */
		block.height = msg->u.reductwork.height;
		block.width = pvtline.width = msg->u.reductwork.width;
		i0 = msg->u.reductwork.i0;
		j0 = msg->u.reductwork.j0;
		message_destroy(msg);

   		/* Receive matrix block. */
		dataGetSpaced(&block.elements, &matrix_segment, OFFSET(matrix_width, i0, j0)*sizeof(float), block.width*sizeof(float), block.height, (block.width+j0)*sizeof(float), NULL);

		start = timer_get();
		row_reduction();
		end = timer_get();
		total += timer_diff(start, end);

		/* Send reduct work. */
		dataPutSpaced(&block.elements, &matrix_segment, OFFSET(matrix_width, i0, j0)*sizeof(float), block.width*sizeof(float), block.height, (block.width+j0)*sizeof(float), NULL);

		/* Send reduct work done signal. */
		postAdd(MPPA_ASYNC_DDR_0, sigback_offset, 1);

		/* Slave cant die yet. Needs to wait another message */
		return 1;

		/* DIE. */
		default:
		message_destroy(msg);
		return 0;	
	}

	mppa_rpc_barrier_all();
}

int main(__attribute__((unused))int argc, const char **argv) {
	/* Initializes async client */
	async_slave_init();

	/* Timer synchronization */
	timer_init();

	/* Util information for the problem. */
	sigback_offset = (off64_t) atoll(argv[0]);
	matrix_width = atoi(argv[1]);
	cid = __k1_get_cluster_id();

	/* Clones message exchange and io_signal segments */
	cloneSegments();

	/* Sends io_signal offset to Master. */
	sendSigOffset();

	/* Slave life. */
	while (doWork());

	/* Put statistics in stats. segment on IO side. */
	sendStatisticsToIO();

	/* Finalizes async library and rpc client */
	async_slave_finalize();

	return 0;
}