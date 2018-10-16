#ifndef MASTER_H_
#define MASTER_H_

/* Kernel Include */
#include <global.h>
#include <message.h>

/* C And MPPA Library Includes*/
#include <mppa_async.h>

#define MATRIX_SEG_0 3

/* Message exchange context */
extern mppa_async_segment_t messages_segment;
extern mppa_async_segment_t signals_segment;
extern struct message works_inProg[NUM_CLUSTERS];
extern off64_t sigOffsets[NUM_CLUSTERS];
extern long long cluster_signals[NUM_CLUSTERS];

/* Matrix blocks exchange. */
extern mppa_async_segment_t matrix_segment;

#endif