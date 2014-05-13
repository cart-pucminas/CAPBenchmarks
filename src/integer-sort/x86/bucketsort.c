/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <limits.h>
#include <omp.h>
#include "integer-sort.h"

/*
 * Bucket sort algorithm.
 */
void bucketsort(int *array, int n)
{
	int max;               /* Max number in array. */
	int i, j;              /* Loop indexes.        */
	int range;             /* Bucket range.        */
	struct list **buckets; /* Buckets.             */
	
	/* Create buckets. */
	buckets = smalloc(nthreads*sizeof(struct list *));
	for (i = 0; i < nthreads; i++)
		buckets[i] = list_create();
	
	/* Find max number in the array. */
	max = INT_MIN;
	#pragma omp parallel for reduction(max : max)
	for (i = 0; i < n; i++)
	{
		/* Found. */
		if (array[i] > max)
			max = array[i];
	}
	
	/* Distribute numbers into buckets. */
	range = max/nthreads;
	for (i = 0; i < n; i++)
	{
		j = array[i]/range;
		if (j >= nthreads)
			j = nthreads - 1;
		
		list_push(buckets[j], array[i]);
	}
	
	/* Sort Each bucket. */
	#pragma omp parallel for shared(buckets, nthreads) private(i)
	for (i = 0; i < nthreads; i++)
		mergesort(buckets[i]);
	
	/* Rebuild array. */
	for (i = 0, j = 0; j < nthreads; j++)
	{
		while (!list_empty(buckets[j]))
			array[i++] = list_pop(buckets[j]);
	}
	
	/* House keeping. */
	for (i = 0; i < nthreads; i++)
		list_destroy(buckets[i]);
	free(buckets);
}
