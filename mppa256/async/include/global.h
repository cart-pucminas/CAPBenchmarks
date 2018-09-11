/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <global.h> - Global variables.
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

/* Kernel Includes */
#include <arch.h>

/* C And MPPA Library Includes*/
#include <stddef.h>
#include <stdint.h>
#include <mppa_async.h>

//	Be verbose?
extern int verbose;

//	Number of clusters to be used.
extern int nclusters;

// Cluster ID.
extern int cid;

/* Timing statistics. */
extern uint64_t master;
extern uint64_t slave[NUM_CLUSTERS];
extern uint64_t communication;
extern uint64_t total;
extern uint64_t spawn;
extern size_t data_sent;
extern size_t data_received;
extern unsigned nsent;
extern unsigned nreceived;

#endif /* GLOBAL_H_ */
