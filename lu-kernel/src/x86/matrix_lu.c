/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * x86/matrix_lu.c - matrix_lu() implementation.
 */

#include <arch.h>
#include <assert.h>
#include <global.h>
#include <math.h>
#include <matrix.h>
#include <omp.h>
#include <util.h>

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
static void _swap_rows(struct matrix *m, int i1, int i2)
{
	int j; /* Loop index. */

#if (_SLOW_SWAP_ == 1)

	float tmp; /* Temporary value. */
	
	/* Swap columns. */
	#pragma omp parallel for private(j, tmp) shared(m, i1, i2)
	for (j = 0; j < m->width; j++)
	{
		tmp = MATRIX(m, i1, j);
		MATRIX(m, i1, j) = MATRIX(m, i2, j);
		MATRIX(m, i2, j) = tmp;
	}

#else

	j = m->i_idx[i1];
	m->i_idx[i1] = m->i_idx[i2];
	m->i_idx[i2] = j;

#endif
}

/*
 * Swaps two columns in a matrix.
 */
static void _swap_columns(struct matrix *m, int j1, int j2)
{
	int i; /* Loop index. */

#if _SLOW_SWAP_ == 1

	float tmp; /* Temporary value. */

	/* Swap columns. */
	#pragma omp parallel for private(i, tmp) shared(m, j1, j2)
	for (i = 0; i < m->height; i++)
	{
		tmp = MATRIX(m, i, j1);
		MATRIX(m, i, j1) = MATRIX(m, i, j2);
		MATRIX(m, i, j2) = tmp;
	}
	
#else

	i = m->j_idx[j1];
	m->j_idx[j1] = m->j_idx[j2];
	m->j_idx[j2] = i;

#endif
}

/*
 * Finds the pivot element.
 */
static float _find_pivot(struct matrix *m, int i0, int j0)
{
	int tid;             /* Thread ID.        */
	int i, j;            /* Loop indexes.     */
	int ipvt[NUM_CORES]; /* Index i of pivot. */
	int jpvt[NUM_CORES]; /* Index j of pivot. */
	
	#pragma omp parallel private(i, j, tid) shared(m, i0, j0, ipvt, jpvt)
	{
		tid = omp_get_thread_num();
	
		ipvt[tid] = i0;
		jpvt[tid] = j0;
	
		/* Find pivot element. */
		#pragma omp for
		for (i = i0; i < m->height; i++)
		{
			for (j = j0; j < m->width; j++)
			{
				/* Found. */
				if (fabs(MATRIX(m, i, j)) < fabs(MATRIX(m,ipvt[tid],jpvt[tid])))
				{
					ipvt[tid] = i;
					jpvt[tid] = j;
				}
			}
		}
	}
	
	/* Min reduction of pivot. */
	for (i = 1; i < nthreads; i++)
	{
		/* Smaller found. */
		if (fabs(MATRIX(m, ipvt[i], jpvt[i])) < fabs(MATRIX(m,ipvt[0],jpvt[0])))
		{
			ipvt[0] = ipvt[i];
			jpvt[0] = jpvt[i];
		}
	}
	
	_swap_rows(m, i0, ipvt[0]);
	_swap_columns(m, j0, jpvt[0]);
	
	return (MATRIX(m, ipvt[0], jpvt[0]));
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
	#pragma omp parallel for private(i, j, mult) shared(m, i0, pivot, j0)
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
int matrix_lu(struct matrix *m, struct matrix *l, struct matrix *u)
{
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
	
	/* Build upper and lower matrixes.  */
	#pragma omp parallel for private(i, j) shared(m, l, u)
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
