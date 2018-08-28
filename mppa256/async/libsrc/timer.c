/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * timer.c - Timer library implementation.
 */

/* Kernel Includes */
#include <arch.h>

/* C And MPPA Library Includes*/
#include <stdint.h>
#include <mOS_vcore_u.h>

/*
 * Timer residual error.
 */
uint64_t timer_error = 0;

/*
 * Gets the current timer value.
 */
uint64_t timer_get(void)
{
	return __k1_read_dsu_timestamp();
}

/*
 * Computers the difference between two timers.
 */
uint64_t timer_diff(uint64_t t1, uint64_t t2)
{
	return (t2 - t1 - timer_error)/MPPA_FREQUENCY;
}

/*
 * Initializes the timer.
 */
void timer_init(void)
{
  uint64_t start, end;
  
  start = timer_get();
  end = timer_get();
  
  timer_error = (end - start);
}


