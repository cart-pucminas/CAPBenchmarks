/*
 * Copyright (C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * <vector/vector_destroy.c> - vector_destroy() implementation.
 */

#include <stdlib.h>
#include <vector.h>

/*
 * Destroys a vector.
 */
void vector_destroy(struct vector *v)
{
	free(v->elements);
	free(v);
}
