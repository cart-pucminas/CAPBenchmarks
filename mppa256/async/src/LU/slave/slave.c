/* Kernel Include */
#include <async_util.h>
#include <message.h>
#include <global.h>
#include <timer.h>
#include "slave.h"

/* C And MPPA Library Includes*/
#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MATRIX_SEG_0 3

/* Message exchange context. */
static mppa_async_segment_t messages_segment;
static mppa_async_segment_t sigOffsets_segment;
static long long signal;

/* Matrix remote segment. */
static mppa_async_segment_t matrix_segment;

/* Individual Slave statistics. */
uint64_t total = 0;          /* Time spent on slave.    */
uint64_t communication = 0;  /* Time spent on comms.    */               
size_t data_put = 0;         /* Number of bytes put.    */
size_t data_get = 0;         /* Number of bytes gotten. */
unsigned nput = 0;           /* Number of put ops.      */
unsigned nget = 0;           /* Number of get ops.      */

/* Timing statistics auxiliars. */
uint64_t start, end;

/* Matrix dimensions information. */
static int m_width, m_height;

/*
 * Returns the element [i][j] of the block.
 */
#define BLOCK(i, j) \
(block.elements[block.width*(i) + (j)])

static void cloneSegments() {
	cloneSegment(&sigOffsets_segment, SIG_SEG_0, 0, 0, NULL);
	cloneSegment(&messages_segment, MSG_SEG_0, 0, 0, NULL);
	cloneSegment(&matrix_segment, MATRIX_SEG_0, 0, 0, NULL);
}

static void sendSigOffset() {
	off64_t offset = 0;
	mppa_async_offset(mppa_async_default_segment(cid), &signal, &offset);
	dataPut(&offset, &sigOffsets_segment, cid, 1, sizeof(off64_t), NULL);
}

static void waitMessageSignal() {
	if (cid == 0)
		waitCondition(&signal, 0, MPPA_ASYNC_COND_GT, NULL);
}

/*
 * Finds the pivot element.
 */
static void _find_pivot(int *ipvt, int *jpvt)
{
	int tid;                             /* Thread ID.        */
	int i, j;                            /* Loop indexes.     */
	int _ipvt[(NUM_CORES/NUM_CLUSTERS)]; /* Index i of pivot. */
	int _jpvt[(NUM_CORES/NUM_CLUSTERS)]; /* Index j of pivot. */
	
	#pragma omp parallel private(i, j, tid) default(shared)
	{
		tid = omp_get_thread_num();
	
		_ipvt[tid] = 0;
		_jpvt[tid] = 0;
	
		/* Find pivot element. */
		#pragma omp for
		for (i = 0; i < block.height; i++)
		{
			for (j = 0; j < block.width; j++)
			{
				/* Found. */
				if (fabs(BLOCK(i, j)) > fabs(BLOCK(_ipvt[tid],_jpvt[tid])))
				{
					_ipvt[tid] = i;
					_jpvt[tid] = j;
				}
			}
		}
	}
	
	/* Min reduction of pivot. */
	for (i = 1; i < (NUM_CORES/NUM_CLUSTERS); i++)
	{
		/* Smaller found. */
		if (fabs(BLOCK(_ipvt[i], _jpvt[i])) > fabs(BLOCK(_ipvt[0],_jpvt[0])))
		{
			_ipvt[0] = _ipvt[i];
			_jpvt[0] = _jpvt[i];
		}
	}
	
	*ipvt = _ipvt[0];
	*jpvt = _jpvt[0];
}

static void testFindWork(struct message *msg) {
	printf("Type = %d || i0 = %d  || j0 = %d || height = %d || width = %d\n", msg->type, msg->u.findwork.i0, msg->u.findwork.j0, msg->u.findwork.height, msg->u.findwork.width);
	fflush(stdout);
}

static int doWork() {
	int i0, j0;    /* Block start.             */
	int ipvt;      /* ith idex of pivot.       */
	int jpvt;      /* jth index of pivot.      */
	size_t n;      /* Number of bytes to send. */
	struct message *msg; /* Message.           */

	/* Waits for message signal to continue. */
	waitCondition(&signal, 1 , MPPA_ASYNC_COND_EQ, NULL);

	/* Get message from messages remote segment. */
	mppa_async_event_t msg_event;
	msg = message_get(&messages_segment, cid, &msg_event);

	/* Making sure that msg get completes before continue. */
	waitEvent(&msg_event);
	
	// LEMBRAR DE RETIRAR O SIZEOF(FLOAT) DO N TODO CASO E PASSAR NO DATA GET
	switch (msg->type)	{
		/* FINDWORK. */
		case FINDWORK:
		//testFindWork(msg);

		/* Receive matrix block. */
		n = (msg->u.findwork.height)*(msg->u.findwork.width);             /* Number of elements.      */
		
		/* Get block from matrix segment. */
		mppa_async_event_t block_event;
		dataGet(&block.elements, &matrix_segment, OFFSET(m_width, msg->u.findwork.i0, msg->u.findwork.j0), n, sizeof(float), &block_event);

		/* Making sure that block get op was completed. */
		waitEvent(&block_event);

		/* Extract message information. */
		block.height = msg->u.findwork.height;
		block.width = msg->u.findwork.width;
		i0 = msg->u.findwork.i0;
		j0 = msg->u.findwork.j0;
		message_destroy(msg);

		
		start = timer_get();
		_find_pivot(&ipvt, &jpvt);
		end = timer_get();
		total += timer_diff(start, end);

		/*
		msg = message_create(FINDRESULT, i0, j0, ipvt, jpvt);
		message_send(outfd, msg);
		message_destroy(msg);*/

		return 0;

		/* REDUCTRESULT. */
		case REDUCTWORK :
		
		return 0;

		/* DIE. */
		default:

		message_destroy(msg);

		return 0;	
	}
}

int main(__attribute__((unused))int argc, const char **argv) {
	/* Initializes async client */
	async_slave_init();

	/* Timer synchronization */
	timer_init();

	/* Util information for the problem. */
	cid = __k1_get_cluster_id();
	m_height = atoi(argv[0]);
	m_width = atoi(argv[1]);

	/* Clones message exchange and signal segments */
	cloneSegments();

	/* Sends signal offset to Master. */
	sendSigOffset();

	/* Slave life. */
	while (doWork());

	/* Finalizes async library and rpc client */
	async_slave_finalize();

	return 0;
}