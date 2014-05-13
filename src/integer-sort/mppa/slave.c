/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * slave.c - slave process.
 */

#include <assert.h>
#include <mppa/osconfig.h>
#include <mppaipc.h>
#include <omp.h>
#include <stdio.h>
#include "integer-sort.h"

/* Inter process communication. */
static int rank;    /* Process rank.   */
static int infd;    /* Input channel.  */
static int sync_fd; /* Sync file descriptor. */

/* Chunk of numbers. */
static int chunk[CHUNK_SIZE];

/*
 * Receives number from master process.
 */
static void receivechunk(void)
{	
	ssize_t count;
	
	count = mppa_read(infd, chunk, CHUNK_SIZE*sizeof(int));
	assert(count != -1);
}

/*
 * Opens NoC connectors.
 */
static void open_noc_connectors(void)
{
	char path[35];
	
	sync_fd = mppa_open("/mppa/sync/128:64", O_WRONLY);
	assert(sync_fd != -1);

	/* Open input channel. */
	sprintf(path, "/mppa/channel/%d:%d/128:%d", rank, rank + 1, rank + 1);
	infd = mppa_open(path, O_RDONLY);
	assert(infd != -1);
}

/*
 * Synchronizes with master process.
 */
static void sync_master(void)
{
	uint64_t mask;
	
	/* Synchronize with master. */
	mask = 1 << rank;
	assert(mppa_write(sync_fd, &mask, 8) == 8);
}

/*
 * Closes NoC connectors.
 */
static void close_noc_connectors(void)
{
	/* Close channels. */
	mppa_close(sync_fd);
	mppa_close(infd);
	
}

#define exch(a, b, t) \
	{ (t) = (a); (a) = (b); (b) = (t); }

#define compexgh(a, b, t)    \
	if ((b) < (a))           \
		exch((b), (a), (t)); \

int M;

static int partition(int *a, int l, int r)
{
	int v;    /* Partitioning element. */
	int t;    /* Temporary element.    */
	int i, j; /* Loop index.           */
	
	i = l - 1;
	j = r;
	v = a[r];
	
	while (1)
	{
		while (a[++i] < v)
			/* NOOP.*/ ;
		
		while (a[--j] > v)
		{
			if (j == l)
				break;
		}
		
		if (i >= j)
			break;
		
		exch(a[i], a[j], t);
	}
	
	exch(a[i], a[r], t);
	
	return (i);
}

static void quicksort(int *a, int l, int r)
{
	int i; /* Pivot.             */
	int t; /* Temporary element. */
	
	/* Fine grain stop. */
	if ((r - l) <= M)
		return;
	
	/* Avoid anomalous partition. */
	exch(a[(l + r) >> 1], a[r - 1], t);
	
	/* Median of three. */
	compexgh(a[l], a[r - 1], t);
	compexgh(a[l], a[r], t);
	compexgh(a[r - 1], a[r], t);
	
	/* Sort. */
	i = partition(a, l + 1 , r - 1);
	quicksort(a, l, i - 1);
	quicksort(a, i + 1, r);
}

static void insertion(int *a, int l, int r)
{
	int t;    /* Temporary value. */
	int v;    /* Working element. */
	int i, j; /* Loop indexes.    */
	
	for (i = r; i > l; i--)
		compexgh(a[i - 1], a[i], t);
	
	for (i = l + 2; i <= r; i++)
	{
		j = i;
		v = a[i];
		
		while (v < a[j - 1])
		{
			a[j] = a[j - 1];
			j--;
		}
		
		a[j] = v;
	}
}

void sort(int *a, int l, int r)
{
	quicksort(a, l, r);
	insertion(a, l, r);
}

/*
 * Sorts a chunk of numbers.
 */
static void integer_sort(void)
{
	int nchunks;
	
	nchunks = PROBLEM_SIZE/CHUNK_SIZE/NUM_PROCS;
	
	while (nchunks-- > 0)
	{
		receivechunk();
		
		fprintf(stderr, "slave %d: sorting chunk %d\n", rank, nchunks + 1);
		
		sort(chunk, 0, PROBLEM_SIZE - 1);
		
		fprintf(stderr, "slave %d: done\n", rank);
	}
}

int main(int argc, char **argv)
{
	((void)argc);
	
	rank = atoi(argv[0]);
	open_noc_connectors();
	sync_master();
	
	
	integer_sort();
	
	close_noc_connectors();
	
	mppa_exit(0);
	return (0);
}
