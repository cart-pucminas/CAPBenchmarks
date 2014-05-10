/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * object/vector.c - Vector object information.
 */

#include <object.h>

/*
 * Vector object information.
 */
const struct objinfo vector =
{
	NULL, /* read()   */
	NULL, /* write()  */
	NULL, /* cmp()    */
	NULL, /* getkey() */
	NULL, /* hash()   */
	NULL  /* clone()  */
};
