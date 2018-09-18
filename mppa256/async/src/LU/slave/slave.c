/* Kernel Include */
#include <async_util.h>
#include <message.h>
#include <global.h>
#include <timer.h>
#include "slave.h"

/* C And MPPA Library Includes*/
#include <stdio.h>

#define ELEM_SEG 50

/* Message exchange context. */
static mppa_async_segment_t messages_segment;
static mppa_async_segment_t sigOffsets_segment;
static struct message msg;
static long long signal;

/* Matrix remote segment. */
static mppa_async_segment_t matrix_segment;

/* Problem information */
int i0, j0;          /* Block start.             */
int ipvt;            /* ith idex of pivot.       */
int jpvt;            /* jth index of pivot.      */
size_t n;            /* Number of bytes to send. */

/* Individual Slave statistics. */
uint64_t total = 0;          /* Time spent on slave.    */
uint64_t communication = 0;  /* Time spent on comms.    */               
size_t data_put = 0;         /* Number of bytes put.    */
size_t data_get = 0;         /* Number of bytes gotten. */
unsigned nput = 0;           /* Number of put ops.      */
unsigned nget = 0;           /* Number of get ops.      */

static void cloneSegments() {
	cloneSegment(&matrix_segment, ELEM_SEG, 0, 0, NULL);
	cloneSegment(&sigOffsets_segment, SIG_SEG_0, 0, 0, NULL);
	cloneSegment(&messages_segment, MSG_SEG_0, 0, 0, NULL);
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

static int doWork() {
	/* Waits for message signal to continue. */
	waitCondition(&signal, 0, MPPA_ASYNC_COND_GT, NULL);

	/* Get message from messages remote segment. */
	dataGet(&msg, &messages_segment, cid, 1, sizeof(struct message), NULL);

	switch (msg.type)	{
		/* FINDWORK. */
		case FINDWORK:
		/* Receive matrix block. */
		n = (msg.u.findwork.height)*(msg.u.findwork.width)*sizeof(float);
		
		return 0;

		/* REDUCTRESULT. */
		case REDUCTWORK :
		
		return 0;

		/* DIE. */
		default:
		
		return 0;	
	}
}

int main(__attribute__((unused))int argc,__attribute__((unused)) char **argv) {
	/* Initializes async client */
	async_slave_init();

	/* Timer synchronization */
	timer_init();

	/* Cluster ID */
	cid = __k1_get_cluster_id();

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