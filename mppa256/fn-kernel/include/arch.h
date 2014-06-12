/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <arch.h> - Architectural contants and definitions.
 */

#ifndef ARCH_H_
#define ARCH_H_

#ifdef _INTEL_I7_

	/*
	 * Number of core on a Intel Core i7 processor.
	 */
	#define NUM_CORES 8

#elif defined(_MPPA_256_)

	#include <mppaipc.h>
	#include <stdlib.h>

	/*
	 * Number of clusters on a MPPA-256 processor.
	 */
	#define NUM_CLUSTERS 16

	/*
	 * Number of IO cores per IO cluster.
	 */
	#define NUM_IO_CORES 4

	/*
	 * Number of cores on a MPPA-256 processor.
	 */
	#define NUM_CORES 256
	
	/*
	 * Frequency of cores.
	 */
	#define MPPA_FREQUENCY 400

#endif

#endif /* ARCH_H_ */
