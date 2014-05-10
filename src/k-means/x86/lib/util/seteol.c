/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * util/seteol.c - seteol() implementation.
 */
 
#include "util.h"

/*
 * Set end of line character.
 */
char seteol(char c)
{
	char old;
	
	old = eol;
	eol = c;
	
	return (old);
}
