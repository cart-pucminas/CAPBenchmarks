/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * master.h -  Private master library.
 */

#ifndef _MASTER_H_
#define _MASTER_H_

	#include <arch.h>
	#include <stdlib.h>
	
	/*
	 * Spwans slave processes.
	 */
	extern void spawn_slaves(void);
	
	/*
	 * Joins slave processes.
	 */
	extern void join_slaves(void);
	
	/*
	 * Waits for slaves to be ready.
	 */
	extern void sync_slaves(void);
	
	/*
	 * Open NoC connectors.
	 */
	extern void open_noc_connectors(void);
	
	/*
	 * Close NoC connectors.
	 */
	extern void close_noc_connectors(void);
	
	/*
	 * Sends data.
	 */
	extern void data_send(int outfd, void *data, size_t n);
	
	/*
	 * Receives data.
	 */
	extern void data_receive(int infd, void *data, size_t n);

	/* Interprocess communication. */
	extern int infd[NUM_CLUSTERS];  /* Input channels.  */
	extern int outfd[NUM_CLUSTERS]; /* Output channels. */

#endif /* _MASTER_H_ */
