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
	
	/* Message types. */
	#define FINDWORK     0 /* Find pivot element. */
	#define FINDRESULT   1 /* Find pivot element. */
	#define REDUCTWORK   2 /* Row reduction.      */
	#define REDUCTRESULT 3 /* Row reduction.      */
	#define DIE          4 /* Die.                */
	
	/*
	 * Message.
	 */
	struct message
	{
		int type; /* Message type (see above). */
		
		union
		{
			/* FINDWORK. */
			struct 
			{
				int i0, j0; /* Block start.  */
				int height; /* Block height. */
				int width;  /* Block width.  */
			} findwork;
			
			/* FINDRESULT. */
			struct
			{
				int ipvt;    /* ith index of pivot. */
				int jpvt;    /* jth index of pivot. */
				int i0, j0;  /* Block start.        */
			} findresult;
			
			/* REDUCTWORK. */
			struct
			{
				int ipvt;   /* Row index of pivot. */
				int i0, j0; /* Block start.        */
				int height; /* Block height.       */
				int width;  /* Block width.        */
			} reductwork;
			
			/* REDUCTRESULT. */
			struct
			{
				int i0, j0; /* Block start.  */
				int height; /* Block height. */
				int width;  /* Block width.  */
				
			} reductresult;
		} u;
		
		struct message *next; /* Next message of a list. */
	};
	
	/*
	 * Receives data.
	 */
	extern void data_receive(int infd, void *data, size_t n);
	
	/*
	 * Sends data.
	 */
	extern void data_send(int outfd, void *data, size_t n);
	
	/*
	 * Creates a message.
	 */
	extern struct message *message_create(int type, ...);
	
	/*
	 * Destroys a message.
	 */
	extern void message_destroy(struct message *msg);
	
	/*
	 * Receives a message.
	 */
	extern struct message *message_receive(int infd);
	
	/*
	 * Sends a message.
	 */
	extern void message_send(int outfd, struct message *msg);
	
	/*
	 * Asserts if a list is empty.
	 */
	#define empty(l)  \
		((l) == NULL) \

	/*
	 * Pushes a message on a list.
	 */
	#define push(l, msg)                \
		{ (msg)->next = (l); (l) = msg; } \

	/*
	 * Pops a message from a list.
	 */
	#define pop(l, msg)                   \
		{ (msg) = (l); (l) = (msg)->next; } \
	
	/* Maximum cluster workload. */
	#define CLUSTER_WORKLOAD 0xfffff /* 1 MB */

	/*
	 * Number of clusters on a MPPA-256 processor.
	 */
	#define NUM_CLUSTERS 16

	/*
	 * Number of cores on a MPPA-256 processor.
	 */
	#define NUM_CORES 256

	/*
	 * Number of IO cores per IO cluster.
	 */
	#define NUM_IO_CORES 4
	
	/*
	 * Frequency of cores.
	 */
	#define MPPA_FREQUENCY 400

#endif

#endif /* ARCH_H_ */
