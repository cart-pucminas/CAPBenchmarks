/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * lu.c - Lower Upper Benchmark Kerkernl.
 */

#include <assert.h>
#include <global.h>
#include <math.h>
#include <omp.h>
#include <util.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "lu.h"
#include "posix.h"
#include "barrier.h"

/*
 * Arguments sanity check.
 */
#define SANITY_CHECK() \
	assert(m != NULL); \
	assert(l != NULL); \
	assert(u != NULL); \

/*
 * Swaps two rows of a matrix.
 */


int *ipvt;
int *jpvt;

sem_t *critical;
sem_t *mutex;
barrier_t* barrier; 

const char *name_object1 = "ipvt";
const char *name_object2 = "jpvt";
const char *name_object3 = "barrier";
const char *name_object4 = "critical";
const char *name_object5 = "mutex";

static void _swap_rows(struct matrix *m, int i1, int i2)
{
	int j;     /* Loop index.      */
	float tmp; /* Temporary value. */
	
	/* Swap columns. */
	for (j = 0; j < m->width; j++)
	{
		tmp = MATRIX(m, i1, j);
		MATRIX(m, i1, j) = MATRIX(m, i2, j);
		MATRIX(m, i2, j) = tmp;
	}
}

/*
 * Swaps two columns in a matrix.
 */
static void _swap_columns(struct matrix *m, int j1, int j2)
{
	int i;     /* Loop index.      */
	float tmp; /* Temporary value. */

	/* Swap columns. */
	for (i = 0; i < m->height; i++)
	{
		tmp = MATRIX(m, i, j1);
		MATRIX(m, i, j1) = MATRIX(m, i, j2);
		MATRIX(m, i, j2) = tmp;
	}
}

/*
 * Finds the pivot element.
 */
static float _find_pivot(struct matrix *m, int i0, int j0)
{
	int i, j; 	/* Loop indexes.          */
//	int ipvt, jpvt;   /* Pivot indexes.         */
	int pipvt, pjpvt; /* Private pivot indexes. */
		
	*ipvt = i0;
	*jpvt = j0;
	
	
	pipvt = i0;
	pjpvt = j0;
		
	wait_barrier(barrier);

	/* Find pivot element. */
	for (i = i0; i < m->height; i++)
	{
		for (j = j0; j < m->width; j++)
		{
			/* Found. */
			if (fabs(MATRIX(m, i, j)) < fabs(MATRIX(m,pipvt,pjpvt)))
			{
				pipvt = i;
				pjpvt = j;
			}
		}
	}
		
		/* Reduct.(Critical) */
	sem_wait(critical);
	if (fabs(MATRIX(m, pipvt, pjpvt) > fabs(MATRIX(m, *ipvt, *jpvt))))
	{
		*ipvt = pipvt;
		*jpvt = pjpvt;
	}
	sem_post(critical);	
	
	wait_barrier(barrier);	

	_swap_rows(m, i0, *ipvt);
	wait_barrier(barrier);
	_swap_columns(m, j0, *jpvt);
	
	wait_barrier(barrier);
	
	return (MATRIX(m, *ipvt, *jpvt));
}

/*
 * Applies the row reduction algorithm in a matrix.
 */
static void _row_reduction(struct matrix *m, int i0, float pivot)
{
	int j0;      /* Starting column. */
	int i, j;    /* Loop indexes.    */
	float mult;  /* Row multiplier.  */
	
	j0 = i0;
		
	/* Apply row redution in some lines. */
	for (i = i0 + 1; i < m->height; i++)
	{
		mult = MATRIX(m, i, j0)/pivot;
	
		/* Store multiplier. */
		MATRIX(m, i, j0) = mult;
	
		/* Iterate over columns. */
		for (j = j0 + 1; j < m->width; j++)
			MATRIX(m, i, j) = MATRIX(m, i, j) - mult*MATRIX(m, i0, j);
	}
}

/*
 * Performs LU factorization in a matrix.
 */
int lower_upper(struct matrix *m, struct matrix *l, struct matrix *u)
{
	
	int nprocs = nthreads;
	/* Shared Memory variables */
	ipvt = open_shared_mem(name_object1,sizeof(int));
	jpvt = open_shared_mem(name_object2,sizeof(int));
	
	
	/* Semaphores */
	barrier = open_shared_mem(name_object3,sizeof(barrier_t));
	init_barrier(barrier,nprocs);
	
	mutex = sem_open(name_object5, O_CREAT, 0644, 1);
	critical = sem_open(name_object4, O_CREAT, 0644, 1);
	
	int i, j;    /* Loop indexes. */
	float pivot; /* Pivot.        */

	/* Apply elimination on all rows. */
	for (i = 0; i < m->height - 1; i++)
	{
		pivot = _find_pivot(m, i, i);		
		/* Impossible to solve. */
		if (pivot == 0.0)
		{
			warning("cannot factorize matrix");
			return (-1);
		}
		
		_row_reduction(m, i, pivot);
	}

	wait_barrier(barrier);

	/* Build upper and lower matrixes.  */
	for (i = 0; i < m->height; i++)
	{
		for (j = 0; j < m->width; j++)
		{
			if (i > j)
			{
				MATRIX(l, i, j) = 0.0;
				MATRIX(u, i, j) = MATRIX(m, i, j);
			}
			
			else if (i < j)
			{	
				MATRIX(l, i, j) = MATRIX(m, i, j);
				MATRIX(u, i, j) = 0.0;
			}
			
			else
			{
				MATRIX(l, i, j) = 1.0;
				MATRIX(u, i, j) = 1.0;
			}
		}
	}
	
	return (0);
}
