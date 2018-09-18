#ifndef MASTER_H_
#define MASTER_H_

/* Kernel Include */
#include <global.h>
#include <message.h>

/* C And MPPA Library Includes*/
#include <mppa_async.h>

#define ELEM_SEG 50

/* Message exchange context */
extern mppa_async_segment_t messages_segment;
extern mppa_async_segment_t signals_segment;
extern struct message works_inProg[NUM_CLUSTERS];
extern off64_t sigOffsets[NUM_CLUSTERS];

/* Elements segment context. */
extern mppa_async_segment_t elements_segment;
extern float elements[NUM_CLUSTERS * CLUSTER_WORKLOAD/sizeof(float)];

/* Slave statistics result */
typedef struct {
	size_t data_put;         /* Number of bytes put.    */
	size_t data_get;         /* Number of bytes gotten. */
	unsigned nput;           /* Number of put op.       */
	unsigned nget;	         /* Number of get op.      */
	uint64_t slave;          /* Time spent on slave.    */
	uint64_t communication;  /* Time spent on comms.    */
} Info;

#endif