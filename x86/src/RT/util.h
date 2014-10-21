/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */
 
#ifndef UTIL_H_
#define UTIL_H_

	#include <stdlib.h>
 
	/*
	 * Prints an error message and exits.
	 */
	extern void error(const char *msg);
	
	/*
	 * Safe malloc().
	 */
	extern void *smalloc(size_t size);
 
#endif /* UTIL_H_ */
