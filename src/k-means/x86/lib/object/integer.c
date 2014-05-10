/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * object/int.c - Integer object information.
 */

#include <object.h>

/*
 * Compares two keys.
 */
static int integer_cmp(void *key1, void *key2)
{
	return (*((int*)key1) - *(((int*)key2)));
}

/*
 * Gets integer key.
 */
static void *integer_getkey(void *obj)
{
	return (obj);
}

/*
 * Computes hash value.
 */
static int integer_hash(void *key)
{
	return (*((int *)key));
}

/*
 * Integer object information.
 */
const struct objinfo integer =
{
	NULL,           /* read()   */
	NULL,           /* write()  */
	integer_cmp,    /* cmp()    */
	integer_getkey, /* getkey() */
	integer_hash,   /* hash()   */
	NULL            /* clone()  */
};
