/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * timer.c - Timer library implementation.
 */

#include <mppa/osconfig.h>
#include <arch.h>
#include <mppaipc.h>
#include <stdint.h>

/*
 * Timer residual error.
 */
uint64_t timer_error = 0;

/*
 * Gets the current timer value.
 */
uint64_t timer_get(void)
{
	return (__k1_io_read64((void *)0x70084040) / MPPA_FREQUENCY);
}

/*
 * Computers the difference between two timers.
 */
uint64_t timer_diff(uint64_t t1, uint64_t t2)
{
	return (t2 - t1 - timer_error);
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

int k1_get_cluster_id(void)
{
	return (__k1_get_cluster_id());
}
