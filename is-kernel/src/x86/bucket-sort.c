/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <darray.h>
#include <global.h>
#include <limits.h>
#include <list.h>
#include <omp.h>
#include <util.h>

#define NBUCKETS 8192

/*
 * Sorts an array of numbers.
 */
extern void sort(struct darray *da);

/*
 * Bucket sort algorithm.
 */
void bucketsort(int *array, int n)
{
	int max;                 /* Max number in array. */
	int maxp;                /* Max private.         */
	int range;               /* Bucket range.        */
	int i, j, k;             /* Loop indexes.        */
	struct darray **buckets; /* Buckets.             */

	/* Create buckets. */
	buckets = smalloc(NBUCKETS*sizeof(struct darray *));
	for (i = 0; i < NBUCKETS; i++)
		buckets[i] = darray_create(n/NBUCKETS);
	
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
	
	/* Distribute numbers into buckets. */
	range = max/NBUCKETS;
	for (i = 0; i < n; i++)
	{
		j = array[i]/range;
		if (j >= NBUCKETS)
			j = NBUCKETS - 1;
		
		darray_append(buckets[j], array[i]);
	}
	
	/* Sort Each bucket. */
	#pragma omp parallel for private(i) default(shared)
	for (i = 0; i < NBUCKETS; i++)
	{
		if (darray_size(buckets[i]) > 0)
			sort(buckets[i]);
	}
	
	
	/* Rebuild array. */
	k = 0;
	for (i = 0; i < NBUCKETS; i++)
	{
		for (j = 0; j < darray_size(buckets[i]); j++)
			array[k] = darray_get(buckets[i], j);
	}
	
	/* House keeping. */
	for (i = 0; i < NBUCKETS; i++)
		darray_destroy(buckets[i]);
	free(buckets);
}
