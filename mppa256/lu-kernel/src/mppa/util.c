/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/util.c - Utility library implementation.
 */

#include <arch.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <mppa/osconfig.h>

/*
 * Prints an error message and exits.
 */
void error(const char *msg)
{
	fprintf(stderr, "error: %s\n", msg);
	exit(EXIT_FAILURE);
}

/*
 * Prints a warning message.
 */
void warning(const char *msg)
{
	fprintf(stderr, "warning: %s\n", msg);
}

/*
 * Safe malloc().
 */
void *smalloc(size_t size)
{
	void *p;
	
	p = malloc(size);
	
	/* Failed to allocate memory. */
	if (p == NULL)
		error("cannot malloc()");
	
	return (p);
}

/*
 * Safe calloc().
 */
void *scalloc(size_t nmemb, size_t size)
{
	void *p;
	
	p = calloc(nmemb, size);
	
	/* Failed to allocate memory. */
	if (p == NULL)
		error("cannot calloc()");
	
	return (p);
}

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
 * Initializes the timer.
 */
void timer_init(void)
{
  uint64_t start, end;
  
  start = timer_get();
  end = timer_get();
  
  timer_error = (end - start);
}

/*
 * Computers the difference between two times
 */
uint64_t timer_diff(uint64_t t1, uint64_t t2)
{
	return (t2 - t1 - timer_error);
}

#define RANDNUM_W 521288629;
#define RANDNUM_Z 362436069;

static unsigned randum_w = RANDNUM_W;
static unsigned randum_z = RANDNUM_Z;

/*
 * Initializes the random number generator.
 */
void srandnum(int seed)
{
	unsigned w, z;

	w = (seed * 104623) & 0xffffffff;
	randum_w = (w) ? w : RANDNUM_W;
	z = (seed * 48947) & 0xffffffff;
	randum_z = (z) ? z : RANDNUM_Z;
}

/*
 * Generates a random number.
 */
unsigned randnum(void)
{
	unsigned u;
	
	/* 0 <= u < 2^32 */
	randum_z = 36969 * (randum_z & 65535) + (randum_z >> 16);
	randum_w = 18000 * (randum_w & 65535) + (randum_w >> 16);
	u = (randum_z << 16) + randum_w;
	
	return u;
}


/*
 * Sends data.
 */
uint64_t data_send(int outfd, void *data, size_t n)
{	
	ssize_t count;
	uint64_t start, end;
	
	start = timer_get();
	count = mppa_write(outfd, data, n);
	end = timer_get();
	assert(count != -1);
	
	return (timer_diff(start, end));
}

/*
 * Receives data.
 */
uint64_t data_receive(int infd, void *data, size_t n)
{	
	ssize_t count;
	uint64_t start, end;
	
	start = timer_get();
	count = mppa_read(infd, data, n);
	end = timer_get();
	assert(count != -1);
	
	return (timer_diff(start, end));
}
