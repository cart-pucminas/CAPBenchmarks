#ifndef MASTER_H_
#define MASTER_H_

/* Kernel Include */
#include <global.h>
#include <message.h>

/* C And MPPA Library Includes*/
#include <mppa_async.h>

#define MATRIX_SEG_0 3
#define INFOS_SEG_0 4

/* Message exchange context */
extern mppa_async_segment_t messages_segment;
extern mppa_async_segment_t signals_segment;
extern struct message works_inProg[NUM_CLUSTERS];
extern off64_t sigOffsets[NUM_CLUSTERS];
extern long long cluster_signals[NUM_CLUSTERS];

/* Elements segment context. */
extern mppa_async_segment_t elements_segment;
extern float *elements;

/* Matrix blocks exchange. */
extern mppa_async_segment_t matrix_segment;

/* Slave statistics result */
typedef struct {
	size_t data_put;         /* Number of bytes put.    */
	size_t data_get;         /* Number of bytes gotten. */
	unsigned nput;           /* Number of put op.       */
	unsigned nget;	         /* Number of get op.      */
	uint64_t slave;          /* Time spent on slave.    */
	uint64_t communication;  /* Time spent on comms.    */
} Info;

/* Statistics information from clusters */
extern mppa_async_segment_t infos_segment;
extern Info infos[NUM_CLUSTERS];

#endif