/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * array/array.c - Array object information.
 */

#include <array.h>
#include <object.h>
#include "array.h"

/*
 * Compares two keys.
 */
static int array_cmp(void *key1, void *key2)
{
	int i;
	struct array *a1;
	struct array *a2;
	
	a1 = key1;
	a2 = key2;
	
	if (a1->info != a2->info)
		return (-1);
	else if (a1->size != a2->size)
		return (-1);
	
	for (i = 0; i < a1->size; i++)
	{
		if (CMP(a1->info, GETKEY(a1->info, ARRAY(a1, i)), GETKEY(a2->info, ARRAY(a2, i))))
			return (-1);
	}
	
	return (0);
}

/*
 * Gets integer key.
 */
static void *array_getkey(void *obj)
{
	return (obj);
}

/*
 * Integer object information.
 */
const struct objinfo array =
{
	NULL,         /* read()   */
	NULL,         /* write()  */
	array_cmp,    /* cmp()    */
	array_getkey, /* getkey() */
	NULL,         /* hash()   */
	NULL          /* clone()  */
};
