/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/matrix_lu/master.c - matrix_lu() master.
 */

#include <arch.h>
#include <assert.h>
#include <global.h>
#include <matrix.h>
#include <util.h>
#include "master.h"

/*
 * Arguments sanity check.
 */
#define SANITY_CHECK() \
	assert(m != NULL); \
	assert(l != NULL); \
	assert(u != NULL); \

/*
 * Finds the pivot element.
 */
extern float find_pivot(struct matrix *m, int i0, int j0);

/*
 * Applies the row reduction algorithm in a matrix.
 */
extern void row_reduction(struct matrix *m, int i0);

/*
 * Performs LU factorization in a matrix.
 */
int matrix_lu(struct matrix *m, struct matrix *l, struct matrix *u)
{
	int i, j;    /* Loop indexes. */
	float pivot; /* Pivot.        */
	
	/* Setup slaves. */
	open_noc_connectors();
	spawn_slaves();
	sync_slaves();
	
	/* Apply elimination on all rows. */
	for (i = 0; i < m->height - 1; i++)
	{
		pivot = find_pivot(m, i, i);	

		/* Impossible to solve. */
		if (pivot == 0.0)
		{
			warning("cannot factorize matrix");
			return (-1);
		}

		row_reduction(m, i);
	}

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
	
	/* House keeping. */
	join_slaves();
	close_noc_connectors();
	
	return (0);
}
