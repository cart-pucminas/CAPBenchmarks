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
	mppa_async_postadd(MPPA_ASYNC_DDR_0, sigback_offset, 1);
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

/* Finds the pivot element. */
static void _find_pivot(int *ipvt) {
	int tid;                             /* Thread ID.        */
	int _ipvt[(NUM_CORES/NUM_CLUSTERS)]; /* Index i of pivot. */

	#pragma omp parallel private(tid) default(shared)
	{
		tid = omp_get_thread_num();

		_ipvt[tid] = 0;

		#pragma omp for
		for (int i = 0; i < (block.height*matrix_width); i += matrix_width) {
			/* Found. */
			if (fabs(BLOCK(i, 0)) > fabs(BLOCK(_ipvt[tid],0)))
					_ipvt[tid] = i;
		}
	}
	
	/* Min reduction of pivot. */
	for (int i = 1; i < (NUM_CORES/NUM_CLUSTERS); i++) {
		/* Smaller found. */
		if (fabs(BLOCK(_ipvt[i], 0)) > fabs(BLOCK(_ipvt[0], 0)))
			_ipvt[0] = _ipvt[i];
	}
	
	*ipvt = _ipvt[0];
}

/* Applies the row reduction algorithm in a matrix. */
void row_reduction(void) {
	element mult;  /* Row multiplier. */
	element pivot; /* Pivot element.  */
	
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
	int ipvt;            /* ith idex of pivot.        */
 	int n;               /* Number of block elements. */
	struct message *msg; /* Message.                  */

	/* Waits for message io_signal to continue. */
	mppa_async_evalcond(&io_signal, 1 , MPPA_ASYNC_COND_EQ, NULL);

	/* Resets io_signal for the next message. */
	io_signal = 0;

	/* Get message from messages remote segment. */
	msg = message_get(&messages_segment, cid, NULL);
	
	switch (msg->type)	{
		/* FINDWORK. */
		case FINDWORK:
		/* Extract message information. */
		block.width = matrix_width;
		block.height = msg->u.findwork.height;
		i0 = msg->u.findwork.i0;
		n = matrix_width*block.height;
		message_destroy(msg);

		/* Get all column elements. */
		mppa_async_get_spaced(&block.elements, &matrix_segment, OFFSET(matrix_width, i0, i0), sizeof(float), block.height, n, NULL);

		//dataGet(&block.elements[i*block.width], &matrix_segment, OFFSET(matrix_width, i0+i, j0), block.width, sizeof(float), NULL);

		start = timer_get();
		_find_pivot(&ipvt);
		ipvt += i0;
		end = timer_get();
		total += timer_diff(start, end);

		/* Send message back to IO. */
		msg = message_create(FINDRESULT, ipvt);
		message_put(msg, &messages_segment, cid, NULL);

		message_destroy(msg);

		/* Send message ready signal to IO. */
		mppa_async_postadd(MPPA_ASYNC_DDR_0, sigback_offset, 1);

		/* Slave cant die yet. Needs to wait another message */
		return 1;

		/* REDUCTRESULT. */
		case REDUCTWORK :
		/* Receive pivot line. */
		dataGet(&pvtline.elements, &matrix_segment, OFFSET(matrix_width, msg->u.reductwork.ipvt, msg->u.reductwork.j0), msg->u.reductwork.width, sizeof(element), NULL);

		/* Extract message information. */
		block.height = msg->u.reductwork.height;
		block.width = pvtline.width = msg->u.reductwork.width;
		i0 = msg->u.reductwork.i0;
		j0 = msg->u.reductwork.j0;
		message_destroy(msg);

   		/* Receive matrix block. */
   		for (int i = 0; i < block.height; i++)
			dataGet(&block.elements[(i*block.width)], &matrix_segment, OFFSET(matrix_width, i0+i, j0), block.width, sizeof(element), NULL);

		start = timer_get();
		row_reduction();
		end = timer_get();
		total += timer_diff(start, end);

		/* Send reduct work. */
		for (int i = 0; i < block.height; i++)
			dataPut(&block.elements[(i*block.width)], &matrix_segment, OFFSET(matrix_width, i0+i, j0), block.width, sizeof(element), NULL);

		/* Send reduct work done signal. */
		mppa_async_postadd(MPPA_ASYNC_DDR_0, sigback_offset, 1);

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