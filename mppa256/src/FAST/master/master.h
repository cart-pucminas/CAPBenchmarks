/*
 * Copyright(C) 2014 Alyson D. Pereira <alyson.deives@outlook.com>, 
 * 					 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * master.h -  Private master library.
 */

#ifndef _MASTER_H_
#define _MASTER_H_

	#include <arch.h>
	#include <stdlib.h>
	
	/*
	 * Maximum thread num per cluster.
	 */
	#define MAX_THREADS (16)
	
	/*
	 * Maximum chunk size.
	 */
	#define CHUNK_SIZE (1024)
	
	/*
	 * Maximum mask size.
	 */
	#define MASK_SIZE (48)
	
	/*
	 * Mask radius.
	 */
	#define MASK_RADIUS (3)
	
	/*
	 * Maximum image size.
	 */
	#define IMG_SIZE (32768)
	
	/* Type of messages. */
	#define MSG_CHUNK 1
	#define MSG_DIE   0
	
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

	/* Interprocess communication. */
	extern int infd[NUM_CLUSTERS];  /* Input channels.  */
	extern int outfd[NUM_CLUSTERS]; /* Output channels. */

#endif /* _MASTER_H_ */
