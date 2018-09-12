/* Kernel Includes */
#include <util.h>
#include "matrix.h"

/* C And MPPA Library Includes*/
#include <stdlib.h>

/* Creates a matrix. */
struct matrix *matrix_create(int height, int width) {

	if ((height <= 0) || (width <= 0))
		error("Matrix dimensions invalid");

	struct matrix *m;
	
	m = smalloc(sizeof(struct matrix));

	/* Initializing matrix. */
	m->height = height;
	m->width = width;
	m->elements = scalloc(height*width, sizeof(float));
	
	return (m);
}

/* Destroys a matrix. */
void matrix_destroy(struct matrix *m) {
	if (m == NULL)
		error("Null Matrix");
	
	free(m->elements);
	free(m);
}

/* Fills up a matrix with random numbers. */
void matrix_random(struct matrix *m) {
	if (m == NULL)
		error("Null Matrix");
	
	/* Fill matrix. */
	for (int i = 0; i < m->height; i++) {
		for (int j = 0; j < m->width; j++)
			MATRIX(m, i, j) = randnum();
	}
}