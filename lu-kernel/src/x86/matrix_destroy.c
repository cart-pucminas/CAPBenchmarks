/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * x86/matrix_destroy.c - matrix_destroy() implementation.
 */

#include <assert.h>
#include <global.h>
#include <matrix.h>
#include <stdlib.h>

/*
 * Arguments sanity check.
 */
#define SANITY_CHECK() \
	assert(m != NULL); \

/*
 * Destroys a matrix.
 */
void matrix_destroy(struct matrix *m)
{
	SANITY_CHECK();
	
	free(m->elements);
	
#if _SLOW_SWAP_ == 0

	free(m->i_idx);
	free(m->j_idx);
	
#endif 

	free(m);
}
