#ifndef MASTER_H_
#define MASTER_H_

/* Kernel Include */
#include <global.h>
#include <message.h>

/* C And MPPA Library Includes*/
#include <mppa_async.h>

/* Message exchange context */
extern mppa_async_segment_t messages_segment;
extern struct message works_inProg[NUM_CLUSTERS];

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