/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * array/array_create.c - array_create() implementation.
 */

#include <array.h>
#include <object.h>
#include <stdlib.h>

/*
 * Creates an array of objects.
 */
struct array *array_create(const struct objinfo *info, int size)
{
	void *objects;
	struct array *a;
	
	/* Invalid argument(s). */
	if (info == NULL)
		goto error0;
	else if (size < 0)
		goto error0;
	
	a = malloc(sizeof(struct array));
	
	/* Failed to allocate array. */
	if (a == NULL)
		goto error0;

	objects = malloc(size*sizeof(void *));
	
	/* Failed to allocate objects. */
	if (objects == NULL)
		goto error1;

	/* Initialize array. */
	a->size = size;
	a->objects = objects;
	a->info = info;
	
	return (a);

error1:
	free(a);
error0:
	return (NULL);
}
