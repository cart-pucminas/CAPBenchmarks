/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * object/pointer.c - Pointer object information.
 */

#include <object.h>

/*
 * Pointer object information.
 */
const struct objinfo pointer =
{
	NULL, /* read()    */
	NULL, /* write()   */
	NULL, /* cmp()     */
	NULL, /* getkey()  */
	NULL, /* hash()    */
	NULL  /* clone()   */
};
