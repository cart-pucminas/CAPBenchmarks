/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * matrix.c - Matrix library.
 */

#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <global.h>
#include <util.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "lu.h"

/*
 * Arguments sanity check.
 */
#define SANITY_CHECK()  \
	assert(height > 0); \
	assert(width > 0);  \
/*
 * Opens a shared memory region(POSIX)
 */


void* open_shared_mem_struct(const char *name_object, int range){
	void *addr;
	int shm_fd = shm_open(name_object, O_CREAT | O_RDWR , 0666);
	if(shm_fd == -1){
		fprintf(stderr, "Open failed:%s\n", strerror(errno));
		exit(1);
	}	
	if(ftruncate(shm_fd,range) == -1){
		perror("ftruncate() error");
		exit(1);
	}	
	addr = mmap(0,range, PROT_READ | PROT_WRITE, MAP_SHARED,shm_fd,0);
	if(addr == (void *)-1){
		fprintf(stderr, "mmap failed : %s\n",strerror(errno));
	        exit(1);
	}
	return addr;	
}



/*
 * Creates a matrix.
 */
struct matrix *matrix_create(int height, int width)
{
	const char *matrix = "matrix";
	struct matrix *m = open_shared_mem_struct(matrix,sizeof(struct matrix)); /* Matrix.     */
	
	SANITY_CHECK();
	
	//m = smalloc(sizeof(struct matrix));

	/* Initialize matrix. */
	m->height = height;
	m->width = width;
	m->elements = scalloc(height*width, sizeof(double));
	
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

	for(i = 0;i < 10; i++){
		for(j = 0;j < 10;j++){
			printf("%f\n",MATRIX(m, i, j));
		}
	}
}	
