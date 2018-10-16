/* Kernel Includes */
#include <async_util.h>
#include <global.h>
#include <timer.h>
#include <util.h>
#include "master.h"
#include "vector.h"

/* C And MPPA Library Includes*/
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* K-means. */
static float mindistance; /* Minimum distance.          */
static int ncentroids;    /* Number of centroids.       */
static int npoints;       /* Number of data points.     */
static int dimension;     /* Dimension of data points.  */
static int *map;          /* Map of clusters.           */
static vector_t *data;    /* Data points.               */
static float *centroids;  /* Data centroids.            */
static int *population;   /* Population of centroids.   */
static float *pcentroids; /* Partial centroids.         */
static int *ppopulation;  /* Partial population.        */
static int *has_changed;  /* Has any centroid changed?  */
static int *too_far;      /* Are points too far?        */
static int *lnpoints;     /* Local number of points.    */
static int *lncentroids;  /* Local number of centroids. */

/* Clusters data. */
int *kmeans(vector_t *_data, int _npoints, int _ncentroids, float _mindistance) {
	/* Initializes async server */
	async_master_start();

	/* Setup parameters. */
	data = _data;
	npoints = _npoints;
	ncentroids = _ncentroids;
	mindistance = _mindistance;
	dimension = vector_size(data[0]);


	_kmeans();

	/* Waiting for PE0 of each cluster to end */
	join_slaves();

	/* Finalizes async server */
	async_master_finalize();
	
	return (map);
}