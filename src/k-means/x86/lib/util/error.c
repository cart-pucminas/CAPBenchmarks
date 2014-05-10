/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * util/error.c - error() implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <util.h>

/*
 * Prints an error message in the standard error file and exits.
 */
void error(char *msg, ...)
{
	va_list args;
	char buffer[512];
	
	va_start(args, msg);
	vsprintf(buffer, msg, args);
	va_end(args);
	
	fprintf(stderr, "Error: %s\n", buffer);
	
	exit(-1);
}
