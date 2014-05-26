/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * x86/matrix_create() implementation.
 */

#include <assert.h>
#include <global.h>
#include <matrix.h>
#include <util.h>

/*
 * Arguments sanity check.
 */
#define SANITY_CHECK()  \
	assert(height > 0); \
	assert(width > 0);  \

/*
 * Creates a matrix.
 */
struct matrix *matrix_create(int height, int width)
{
	struct matrix *m; /* Matrix.     */
	
#if _SLOW_SWAP_ == 0

	int i;      /* Loop index.     */
	int *i_idx; /* Row indexes.    */
	int *j_idx; /* Column indexes. */

#endif
	
	SANITY_CHECK();
	
	m = smalloc(sizeof(struct matrix));

	/* Initialize matrix. */
#if _SLOW_SWAP_ == 0

	i_idx = smalloc(height*sizeof(int));
	j_idx = smalloc(width*sizeof(int));
	
	for (i = 0; i < height; i++)
		i_idx[i] = i;
	for (i = 0; i < width; i++)
		j_idx[i] = i;
		
	m->i_idx = i_idx;
	m->j_idx = j_idx;

#endif
	m->height = height;
	m->width = width;
	m->elements = scalloc(height*width, sizeof(float));
	
	return (m);
}
