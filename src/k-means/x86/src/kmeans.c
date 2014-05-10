/*
 * Copyright(C) 2013-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * algorithm/kmeans.c - kmeans() implementation.
 */

#include <array.h>
#include <globl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector.h>
#include <string.h>
#include <util.h>

#include <omp.h>

#include "random.h"


/* extern int myrand(void); */
extern inline unsigned int simple_rng_rand();

/*
 * Kmeans data.
 */
struct kmeans_data
{
	/* General info. */
	array_t data;        /* Data being clustered.                       */
	int *map;            /* Data map.                                   */
	int npoints;         /* Number of data points.                      */
	vector_t *centroids; /* Data centroids.                             */
	int ncentroids;      /* Number of clusters.                         */
	float mindistance;  /* Minimum distance.                           */
	int dimension;       /* Data dimension.                             */
	int again;           /* Keep iterating.                             */
	int *too_far;        /* Is there any points too far from centroids? */
	int *has_changed;    /* has any centroid change?                    */
	vector_t *tmp;       /* Temporary centroids.                        */
};

void print_state(struct kmeans_data *k, int iteration);

/*
 * Populates clusters.
 */
static void populate(struct kmeans_data *k)
{
	int i, j;              /* Loop indexes.       */
	float tmp;            /* Auxiliary variable. */
	float distance;       /* Distance.           */
	
	for (i = 0; i < nthreads; i++)
		k->too_far[i] = 0;
	
	/* Iterate over data points. */
	#pragma omp parallel for schedule(static) default(shared) private(i, j, tmp, distance)
	for (i = 0; i < k->npoints; i++)
	{	
		distance = vector_distance(k->centroids[k->map[i]], ARRAY(k->data, i));
		
		/* Look for closest cluster. */
		for (j = 0; j < k->ncentroids; j++)
		{	
			/* Point is in this cluster. */
			if (j == k->map[i])
				continue;
				
			tmp = vector_distance(k->centroids[j], ARRAY(k->data, i));
			
			/* Found. */
			if (tmp < distance)
			{
				k->map[i] = j;
				distance = tmp;
			}
		}
		
		/* Cluster is too far away. */
		if (distance > k->mindistance)
			k->too_far[omp_get_thread_num()] = 1;
	}
}

static void vector_clear(vector_t v)
{
	memset(v->elements, 0, sizeof(float)*vector_size(v));	
}

/*
 * Computes cluster's centroids.
 */
static void compute_centroids(struct kmeans_data *k)
{
	int i, j;       /* Loop indexes.       */
	int population; /* Cluster population. */
	vector_t tmp;
	
	for (i = 0; i < nthreads; i++)
		k->has_changed[i] = 0;
	
	/* Compute means. */
	#pragma omp parallel for schedule(static) default(shared) private(i, j, population, tmp)
	for (i = 0; i < k->ncentroids; i++)
	{	
		tmp = k->tmp[omp_get_thread_num()];
		
		/* Initialize temporary vector.*/
		vector_assign(tmp, k->centroids[i]);
		vector_clear(k->centroids[i]);
		
		/* Compute cluster's mean. */
		population = 0;
		for (j = 0; j < k->npoints; j++)
		{
			/* Not a member of this cluster. */
			if (k->map[j] != i)
				continue;			
			
			vector_add(k->centroids[i], ARRAY(k->data, j));
				
			population++;
		}		
		if (population > 1)
			vector_mult(k->centroids[i], 1.0/population);
			
		/* Cluster mean has changed. */
		if (!vector_equal(tmp, k->centroids[i]))
			k->has_changed[omp_get_thread_num()] = 1;
	}
}

/*
 * Initializes k-means.
 */
static struct kmeans_data *kmeans_init(array_t data, int ncentroids, float mindistance)
{
	int i, p;               /* Loop indexes.       */
	int npoints;            /* Number of points.   */
	int *map;               /* Data map.           */
	int dimension;          /* Data dimension.     */
	vector_t *tmp;          /* Temporary centroid. */
	vector_t *centroids;    /* Data centroids.     */
	struct kmeans_data *k;  /* K-means data.       */
	
	npoints = array_size(data);
	dimension = vector_size(ARRAY(data, 0));
	
	k = malloc(sizeof(struct kmeans_data));
	
	/* Failed to allocate k-means data. */
	if (k == NULL)
		goto error0;
	
	map = calloc(npoints, sizeof(int));
	
	/* Failed to allocate data map. */
	if (map == NULL)
		goto error1;
		
	centroids = malloc(ncentroids*sizeof(vector_t));
	
	/* Failed to allocate centroids. */
	if (centroids == NULL)
		goto error2;
	
	/* Create centroids. */
	for (i = 0; i < ncentroids; i++)
	{
		centroids[i] = vector_create(dimension);
		
		/* Failed to create centroid. */
		if (centroids[i] == NULL)
			goto error3;
	}
	
	tmp = malloc(nthreads*sizeof(vector_t));
	
	/* Failed to allocate temporary centroids. */
	if (tmp == NULL)
		goto error4;
	
	/* Create temporary centroids. */
	for (i = 0; i < nthreads; i++)
	{
		tmp[i] = vector_create(dimension);
		
		/* Failed to create temporary centroid. */
		if (tmp[i] == NULL)
			goto error5;
	}

	/* Instantiate too_far and has_changed arrays */
	k->too_far = malloc(nthreads * sizeof(int));
	k->has_changed = malloc(nthreads * sizeof(int));
	
	/* Initialize k-means data. */
	k->data = data;
	k->map = map;
	k->npoints = npoints;
	k->centroids = centroids;
	k->ncentroids = ncentroids;
	k->mindistance = mindistance;
	k->dimension = dimension;
	k->tmp = tmp;
	
	/* Generate random data points. */
	for (i = 0; i < npoints; i++)
	{
		map[i] = -1;
	}
	
	/* Initialize centroids. */
	for (i = 0; i < ncentroids; i++)
	{
		vector_assign(k->centroids[i], ARRAY(data, p = simple_rng_rand() % npoints));
		k->map[p] = i;
	}
	for (i = 0; i < npoints; i++)
	{
		if (map[i] < 0)
		  map[i] = simple_rng_rand() % ncentroids;
	}
	
	return (k);

error5:
	while(i-- > 0)
		vector_destroy(tmp[i]);
	free(tmp[i]);
error4:
	i = ncentroids;
error3:
	while(i-- > 0)
		vector_destroy(centroids[i]);
	free(centroids);
error2:
	free(map);
error1:
	free(k);
error0:
	return (NULL);
}

/*
 * Ends k-means.
 */
int *kmeans_end(struct kmeans_data *k)
{
	int i;    /* Loop index. */
	int *map; /* Data map.   */
		
	map = k->map;
	
	/* House keeping. */
	for (i = 0; i < nthreads; i++)
		vector_destroy(k->tmp[i]);
	free(k->tmp);
	for (i = 0; i < k->ncentroids; i++)
		vector_destroy(k->centroids[i]);
	free(k->centroids);
	free(k->too_far);
	free(k->has_changed);
	free(k);
	
	return (map);
}

/*
 * Clusters data. 
 */
int *kmeans(array_t data, int ncentroids, float mindistance)
{
	
	int it;                /* Number of iterations. */
	struct kmeans_data *k; /* K-means data.         */
	int i, again;

	
	setvbuf(stderr, NULL, _IONBF, 0);

	omp_set_num_threads(nthreads);
	
	k = kmeans_init(data, ncentroids, mindistance);
	
	/* Failed to initialize k-means. */
	if (k == NULL)
		return (NULL);
	
	it = 0;
	
	/* Cluster data. */
	do
	{
		it++;
		fflush(stderr);
		populate(k);
		fflush(stderr);
		compute_centroids(k);

		/* Check if we need to loop. */
		for (i = 0; i < nthreads; i++)
		{
			/* We will need another iteration. */
			if (k->too_far[i] && k->has_changed[i])
				break;		
		}

		again = (i < nthreads) ? 1 : 0;

	} while (again);

	if (verbose)
		fprintf(stderr, "Done in %d iterations\n", it);
	
	printf("%d;", it);
		
	return (kmeans_end(k));
}

void print_state(struct kmeans_data *k, int iteration) {
	int i;

	printf("Iteration %d:\n", iteration);
	printf("\thas_changed=[");
	for (i = 0; i < nthreads - 1; i++)
		printf("%d,", k->has_changed[i]);
	printf("%d]\n", k->has_changed[i]);

	printf("\ttoo_far    =[");
	for (i = 0; i < nthreads - 1; i++)
		printf("%d,", k->too_far[i]);
	printf("%d]\n\n", k->too_far[i]);
}
