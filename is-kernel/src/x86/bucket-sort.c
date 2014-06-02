/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <global.h>
#include <limits.h>
#include <list.h>
#include <omp.h>
#include <util.h>

#define NBUCKETS 8192

/*
 * Merge sort algorithm.
 */
extern void mergesort(struct list *l);

/*
 * Bucket sort algorithm.
 */
void bucketsort(int *array, int n)
{
	int max;               /* Max number in array. */
	int maxp;              /* Max private.         */
	int i, j;              /* Loop indexes.        */
	int range;             /* Bucket range.        */
	struct list **buckets; /* Buckets.             */

	/* Create buckets. */
	buckets = smalloc(NBUCKETS*sizeof(struct list *));
	for (i = 0; i < NBUCKETS; i++)
		buckets[i] = list_create();
	
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
		
		list_push(buckets[j], array[i]);
	}
	
	/* Sort Each bucket. */
	#pragma omp parallel for private(i) default(shared)
	for (i = 0; i < NBUCKETS; i++)
	{
		if (!list_empty(buckets[i]))
			mergesort(buckets[i]);
	}
	
	/* Rebuild array. */
	i = 0;
	for (j = 0; j < NBUCKETS; j++)
	{
		while (!list_empty(buckets[j]))
			array[i++] = list_pop(buckets[j]);
	}
	
	/* House keeping. */
	for (i = 0; i < NBUCKETS; i++)
		list_destroy(buckets[i]);
	free(buckets);
}
