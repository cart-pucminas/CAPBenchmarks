/*
 * Copyright(C) 2019 Guilherme Reis Barbosa de Oliveira <grboliveira@sga.pucminas.br>
 * 
 * lu.c - Lower Upper Benchmark Kerkernl.
 */

#include <assert.h>
#include <global.h>
#include <util.h>
#include <stdio.h>
#include <semaphore.h>
#include "posix.h"
#include "lu.h"

/*
 * Arguments sanity check.
 */
#define SANITY_CHECK()  \
	assert(height > 0); \
	assert(width > 0);  \

/*
 * Creates a matrix.
 */
struct matrix *matrix_create(int height, int width,const char *mem_name,const char *elem_name)
{
	struct matrix *m; /* Matrix.     */

	SANITY_CHECK();
	
	m = open_shared_mem(mem_name,sizeof(struct matrix)); 
	/* Initialize matrix. */
	m->height = height;
	m->width = width;
	m->elements = open_shared_mem(elem_name,(height * width) * sizeof(float));
	return (m);
}

#undef SANITY_CHECK

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

	free(m);
}

#undef SANITY_CHECK

/*
 * Arguments sanity check.
 */
#define SANITY_CHECK() \
	assert(m != NULL); \

/*
 * Fills up a matrix with random numbers.
 */
void matrix_random(struct matrix *m)
{
	int i, j;
	
	SANITY_CHECK();	
	/* Fill matrix. */
	for (i = 0; i < m->height; i++)
	{
		for (j = 0; j < m->width; j++)
			MATRIX(m, i, j) = randnum();
	}
}

void matrix_show(struct matrix *m){
	int i , j;

	for(i = 0;i < m->height; i++){
		for(j = 0;j < m->width;j++){
			printf("%f",MATRIX(m, i, j));
		}
	}
}	
