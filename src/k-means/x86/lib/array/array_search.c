/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * array/array_search.c - array_search() implementation.
 */

#include <array.h>
#include <object.h>
#include <stdlib.h>

/*
 * Searches for an object in an array of objects.
 */
int array_search(struct array *a, void *key)
{
	int i;
	
	/* Cant search in array. */
	if (a->info->getkey == NULL)
		return (-1);
	else if (a->info->cmp == NULL)
		return (-1);
	
	/* Sequential search. */
	for (i = 0; i < a->size; i++)
	{
		/* Found. */
		if (!CMP(a->info, GETKEY(a->info, ARRAY(a, i)), key))
			return (i);
	}
	
	return (-1);
}
