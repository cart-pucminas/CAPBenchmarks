/* Kernel Include */
#include <global.h>
#include <util.h>
#include "km.h"

/* C And MPPA Library Includes*/
#include <math.h>
#include <omp.h>
#include <string.h>

#include <stdio.h>

/* Kmeans data. */
int dimension ;    /* Problem dimension         */
int npoints;       /* Number of points.         */
int ncentroids;    /* Number of clusters.       */
int *map;          /* Map of clusters.          */
float *points;     /* Data being clustered.     */
float *centroids;  /* Data centroids.           */
int *populations;  /* Clusters population.      */
int *has_changed;  /* has any centroid change?  */

static omp_lock_t *lock;

static void populate() {
	int tid;        /* Thread ID.             */
	int i, j;       /* Loop indexes.          */
	int lock_aux;   /* Lock auxiliar.         */
	int init_map;   /* Point initial mapping. */
	float tmp_dist; /* Temporary distance.    */
	float distance; /* Distance.              */

	/* Reset variables for new calculation. */
	memset(populations, 0, ncentroids*sizeof(int));
	memset(has_changed, 0, nthreads*sizeof(int));

	/* Iterate over data points. */
	#pragma omp parallel private(i, j, tmp_dist, distance, tid, init_map, lock_aux) default(shared)
	{
		tid = omp_get_thread_num();

		#pragma omp for
		for (i = 0; i < npoints; i++) {	
			distance = vector_distance(CENTROIDS(map[i]), POINTS(i));
			init_map = map[i];

			/* Looking for closest cluster. */
			for (j = 0; j < ncentroids; j++) {	
				/* Point is in this cluster. */
				if (j == map[i])
					continue;
						
				tmp_dist = vector_distance(CENTROIDS(j), POINTS(i));
					
				/* Found. */
				if (tmp_dist < distance) {
					map[i] = j;
					distance = tmp_dist;
				}
			}

			lock_aux = map[i] % nthreads;

			omp_set_lock(&lock[lock_aux]);
			populations[map[i]]++;
			omp_unset_lock(&lock[lock_aux]);

			if (map[i] != init_map)
				has_changed[tid] = 1;
		}
	}
}

static void compute_centroids() {
	int i;        /* Loop index.                 */
	int lock_aux; /* Lock auxiliar.              */

	/* Clear all centroids for recalculation. */
	memset(CENTROIDS(0), 0, ncentroids*dimension*sizeof(float));

	/* Compute means. */
	#pragma omp parallel private(i, lock_aux) default(shared) 
	{	
		/* Computing centroids means. */
		#pragma omp for 
		for (i = 0; i < npoints; i++) {
			lock_aux = map[i] % nthreads;
			omp_set_lock(&lock[lock_aux]);
			vector_add(CENTROIDS(map[i]), POINTS(i));
			omp_unset_lock(&lock[lock_aux]);	
		}

		#pragma omp barrier

		/* Computing centroids means. */
		#pragma omp for
		for (i = 0; i < ncentroids; i++) {
			if (populations[i] > 1)
				vector_mult(CENTROIDS(i), 1.0/populations[i]);
		}
	}
}

/* Sets centroids and begins to map points. */
static void init_mapping() {
	int i, j;  /* Loop index. */

	/* Initialize mapping. */
	for (i = 0; i < npoints; i++)
		map[i] = -1;

	/* Initialize centroids. */
	for (i = 0; i < ncentroids; i++) {
		j = randnum()%npoints;
		vector_assign(CENTROIDS(i), POINTS(j));
		map[j] = i;
	}

	/* Map unmapped data points. */
	for (i = 0; i < npoints; i++) {
		if (map[i] < 0)
			map[i] = randnum()%ncentroids;
	}
}

/* Clusters data. */
int *kmeans(float *_points, int _npoints, int _ncentroids, int _dimension) {
	int i;       /* Loop index. */
	int again;   /* Loop again? */

	/* Setup parameters. */
	points = _points;
	npoints = _npoints;
	ncentroids = _ncentroids;
	dimension = _dimension;

	/* Allocate memory for auxiliary structures. */
	map  = scalloc(npoints, sizeof(int));
	centroids = smalloc(ncentroids*dimension*sizeof(float));
	populations = smalloc(ncentroids*sizeof(int));
	lock = smalloc (nthreads*sizeof(omp_lock_t));
	has_changed = smalloc(nthreads*sizeof(int));

	/* Initialize mapping. */
	init_mapping();

	omp_set_num_threads(nthreads);
	for (i = 0; i < nthreads; i++)
		omp_init_lock(&lock[i]);

	/* Iterate over points until all clusters are defined. */
	do {
		populate();
		compute_centroids();

		for (i = 0; i < nthreads; i++){
			if (has_changed[i] == 1)
				break;
		}

		again = (i < nthreads) ? 1 : 0;
	} while (again);

	/* House keeping. */
	free(populations);
	free(centroids);
	free(has_changed);

	return map;
}