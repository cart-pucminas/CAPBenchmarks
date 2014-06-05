/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <util.h> - Utility library.
 */

#ifndef UTIL_H_
#define UTIL_H_
	
	#include <stdint.h>
	#include <stdlib.h>
	
	/* Timing units. */
	#define MICROSEC 0.000001 /* Micro seconds. */

	/*
	 * Prints an error message and exits.
	 */
	extern void error(const char *msg);
	
	/*
	 * Generates a random number.
	 */
	extern unsigned randnum(void);
	
	/*
	 * Safe calloc().
	 */
	extern void *scalloc(size_t nmemb, size_t size);
	
	/*
	 * Safe realloc().
	 */
	extern void *srealloc(void *p, size_t size);
	
	/*
	 * Safe malloc().
	 */
	extern void *smalloc(size_t size);
	
	/*
	 * Initializes the random number generator.
	 */
	extern void srandnum(int seed);
	
	/*
	 * Computers the difference between two times
	 */
	extern uint64_t timer_diff(uint64_t t1, uint64_t t2);
	
	/*
	 * Gets the current timer value
	 */
	extern uint64_t timer_get(void);	
	
	/*
	 * Initializes the timer.
	 */
	extern void timer_init(void);

#endif /* UTIL_H_ */
