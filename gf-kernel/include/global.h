/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <global.h> - Global constants and variables.
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

	#include <arch.h>
	#include <stdint.h>

	/*
	 * pi
	 */
	#define PI 3.14159265359
	
	/*
	 * e
	 */
	#define E 2.71828182845904
	
	/*
	 * Standard deviation.
	 */
	#define SD 0.8
	
	/*
	 * Maximum chunk size.
	 */
	#define CHUNK_SIZE (1024)
	
	/*
	 * Maximum mask size.
	 */
	#define MASK_SIZE (20)
	
	/*
	 * Maximum image size.
	 */
	#define IMG_SIZE (32768)
	
	/* Type of messages. */
	#define MSG_CHUNK 1
	#define MSG_DIE   0
	
	/*
	 * Number of threads.
	 */
	extern int nthreads;
	
	/* Timing statistics. */
#ifdef _MPPA_256_
	extern uint64_t master;
	extern uint64_t slave[NUM_CLUSTERS];
	extern uint64_t communication;
#endif
	extern uint64_t total;

#endif /* GLOBAL_H_ */
