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
	 * Balances workload.
	 */
	void balance(int *work, int n, int k);

	/*
	 * Prints an error message and exits.
	 */
	extern void error(const char *msg);
	
	/*
	 * Safe malloc().
	 */
	extern void *smalloc(size_t size);
	
	/*
	 * Sorts an array of numbers.
	 */
	extern void sort(int *a, int n);
	
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
	
	/*
	 * Sends data.
	 */
	extern uint64_t data_send(int outfd, void *data, size_t n);
	
	/*
	 * Receives data.
	 */
	extern uint64_t data_receive(int infd, void *data, size_t n);

#endif /* UTIL_H_ */
