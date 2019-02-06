/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * util.c - Utility library implementation.
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

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
 * Safe realloc().
 */
void *srealloc(void *p, size_t size)
{
	
	p = realloc(p, size);
	
	/* Failed to allocate memory. */
	if (p == NULL)
		error("cannot realloc()");
	
	return (p);
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
	
	return (u);
}

/*
 * Generates a normal number.
 */
double normalnum(double mu, double sigma)
{
	double U1, U2, W, mult;
	static double X1, X2;
	static int call = 0;
 
	if (call == 1)
	{
		call = !call;
		return (mu + sigma * (double) X2);
	}
 
	do
	{
		U1 = -1 + ((double) randnum() / RAND_MAX) * 2;
		U2 = -1 + ((double) randnum() / RAND_MAX) * 2;
		W = pow (U1, 2) + pow (U2, 2);
	} while (W >= 1 || W == 0);
 
	mult = sqrt ((-2 * log (W)) / W);
	X1 = U1 * mult;
	X2 = U2 * mult;
 
	call = !call;
 
	return (mu + sigma * (double) X1);
}

/*
 * pRNG based on http://www.cs.wm.edu/~va/software/park/park.html
 *
 * Returns a pseudo-random real number uniformly distributed 
 * between 0.0 and 1.0. 
 */
#define MODULUS    2147483647
#define MULTIPLIER 48271

static long prngseed = 0;
double prngnum()
{
  const long Q = MODULUS / MULTIPLIER;
  const long R = MODULUS % MULTIPLIER;
        long t;

  t = MULTIPLIER * (prngseed % Q) - R * (prngseed / Q);
  if (t > 0) 
    prngseed = t;
  else 
    prngseed = t + MODULUS;
  return ((double) prngseed / MODULUS);
}

void prng_set_seed(int seed)
{
	prngseed = seed;
}