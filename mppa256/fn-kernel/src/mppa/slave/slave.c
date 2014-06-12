/* 
 * File:   master.c - Mutually Friendly Numbers master process.
 * Authors: 
 *		Matheus A. Souza <matheusalcantarasouza@gmail.com>
 *      Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * Copyright(C) 2014
 *
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <mppaipc.h>
#include <omp.h>
#include <stdint.h>
#include <stdlib.h>
#include <util.h>

typedef struct {
    long int number;
    long int numerator;
    long int denominator;
} Item;

/* Inter process communication. */
static int rank; /* Process rank.   */
static int infd; /* Input channel.  */
static int outfd; /* Output channel. */

static Item task[65536];

static int tasksize;

static void syncNumbers(void)
{
    data_send(outfd, &task, tasksize*sizeof(Item));
}

static void getwork(void)
{
	data_receive(infd, &tasksize, sizeof(int));
	data_receive(infd, &task, tasksize*sizeof(Item));
}

/*
 * Computes the Greatest Common Divisor of two numbers.
 */
static int gcd(int a, int b)
{
  int c;
  
  /* Compute greatest common divisor. */
  while (a != 0)
  {
     c = a;
     a = b%a;
     b = c;
  }
  
  return (b);
}

/*
 * Some of divisors.
 */
static int sumdiv(int n)
{
	int sum;    /* Sum of divisors. */
	int factor; /* Working factor.  */
	
	sum = 1 + n;
	
	/* Compute sum of divisors. */
	for (factor = 2; factor < n; factor++)
	{
		/* Divisor found. */
		if ((n%factor) == 0)
			sum += factor;
	}
	
	return (sum);
}

/*
 * Computes friendly numbers.
 */
void friendly_numbers(void) 
{
	int n; /* Divisor.      */
	int i; /* Loop indexes. */
	
	/* Compute abundances. */
	#pragma omp parallel for private(i, n) default(shared)
	for (i = 0; i < tasksize; i++) 
	{		
		task[i].numerator = sumdiv(task[i].number);
		task[i].denominator = i;
				
		n = gcd(task[i].numerator, task[i].denominator);
		task[i].numerator /= n;
		task[i].denominator /= n;
	}
}


int main(int argc, char **argv)
{
    char path[35];
    uint64_t mask;       /* Mask for sync.        */
    int sync_fd;         /* Sync file descriptor. */
	uint64_t total;      /* Total time.           */
	uint64_t start, end; /* Timing statistics.    */
	
	timer_init();

    ((void) argc);
    
    total = 0;
    
    rank = atoi(argv[0]);

    sync_fd = mppa_open("/mppa/sync/128:64", O_WRONLY);
    assert(sync_fd != -1);

    /* Open channels. */
    sprintf(path, "/mppa/channel/%d:%d/128:%d", rank, rank + 1, rank + 1);
    infd = mppa_open(path, O_RDONLY);
    assert(infd != -1);
    sprintf(path, "/mppa/channel/128:%d/%d:%d", rank + 33, rank, rank + 33);
    outfd = mppa_open(path, O_WRONLY);
    assert(outfd != -1);
    
    // Synchronize with master.
    mask = 1 << rank;
    assert(mppa_write(sync_fd, &mask, 8) == 8);
	mppa_close(sync_fd);
	
    getwork();

	start = timer_get();
    friendly_numbers();
	end = timer_get();

    syncNumbers();

	total = timer_diff(start, end);
	data_send(outfd, &total, sizeof(uint64_t));

    /* Close channels. */
    mppa_close(infd);
    mppa_close(outfd);

    mppa_exit(0);
    return (0);
}
