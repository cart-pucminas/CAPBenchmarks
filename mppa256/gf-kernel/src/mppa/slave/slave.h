/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * slave.h -  Private slave library.
 */

#ifndef _SLAVE_H_
#define _SLAVE_H_

	#include <stdlib.h>

	/*
	 * Synchronizes with master process.
	 */
	extern void sync_master(void);
	
	/*
	 * Opens NoC connectors.
	 */
	extern void open_noc_connectors(void);
	
	/*
	 * Closes NoC connectors.
	 */
	extern void close_noc_connectors(void);

	/* Inter process communication. */
	extern int rank;  /* Process rank.   */
	extern int infd;  /* Input channel.  */
	extern int outfd; /* Output channel. */
	
#endif /* _SLAVE_H_ */
