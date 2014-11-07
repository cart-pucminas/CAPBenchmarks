/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <global.h>
#include <limits.h>
#include <omp.h>
#include <util.h>
#include "is.h"
#include <stdio.h>

/* Number of buckets. */
#define NUM_BUCKETS 8192

/*
 * Finds the maximum element in an array.
 */
static int findmax(int *array, int n)
{
	int i;    /* Loop index.          */
	int max;  /* Max number in array. */
	int maxp; /* Max private.         */
	
	/* Find max number in the array. */
	max = INT_MIN;
	#pragma omp parallel private(i, maxp) default(shared)
	{
		maxp = INT_MIN;
		
		#pragma omp for
		for (i = 0; i < n; i++)
		{
			/* Found. */
			if (array[i] > max)
				max = array[i];
		}
	
		/* Reduce. */
		if (maxp > max)
		{
			#pragma omp critical
			{
				if (maxp > max)
					max = maxp;
			}
		}
	}
	
	return (max);
}

/*
 * Bucket sort algorithm.
 */
void integer_sort(int *array, int n)
{
	int max;                 /* Max number in array. */
	int range;               /* Bucket range.        */
	int i, j, k;             /* Loop indexes.        */
	int *indexes;            /* Index for buckets.   */
	struct darray **buckets; /* Buckets.            */

	indexes = smalloc(NUM_BUCKETS*sizeof(int));

	/* Create buckets. */
	buckets = smalloc(NUM_BUCKETS*sizeof(struct darray *));
	for (i = 0; i < NUM_BUCKETS; i++)
		buckets[i] = darray_create(n/NUM_BUCKETS);
	
	max = findmax(array, n);
	range = max/NUM_BUCKETS;
	
	#pragma omp parallel private(i, j, k) default(shared)
	{	
		/* Distribute numbers into buckets. */
		#pragma omp master
		for (i = 0; i < n; i++)
		{
			j = array[i]/range;
			if (j >= NUM_BUCKETS)
				j = NUM_BUCKETS - 1;

			darray_append(buckets[j], array[i]);
		}
		
		/* Sort Each bucket. */
		#pragma omp for schedule(dynamic)
		for (i = 0; i < NUM_BUCKETS; i++)
		{
			if (darray_size(buckets[i]) > 0)
				sort(buckets[i]);
		}
		
		#pragma omp master
		{
			/* Build indexes. */
			indexes[0] = 0;
			for (i = 1; i < NUM_BUCKETS; i++)
				indexes[i] = indexes[i - 1] + darray_size(buckets[i]);
		
			/* Rebuild array. */
			for (i = 0; i < NUM_BUCKETS; i++)
			{
				k = indexes[i];
					
				for (j = 0; j < darray_size(buckets[i]); j++)
					array[k + j] = darray_get(buckets[i], j);
			}
		}
	}
	
	/* House keeping. */
	for (i = 0; i < NUM_BUCKETS; i++)
		darray_destroy(buckets[i]);
	free(buckets);
	free(indexes);
}
