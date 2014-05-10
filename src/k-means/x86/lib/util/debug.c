/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * util/debug.c - debug() implementation.
 */

#include <stdio.h>
#include <stdarg.h>
#include <util.h>

/*
 * Prints a debug message in the standard error file .
 */
void debug(char *msg, ...)
{
	va_list args;
	char buffer[512];
	
	va_start(args, msg);
	vsprintf(buffer, msg, args);
	va_end(args);
	
	fprintf(stderr, "Debug: %s\n", buffer);
	fflush(stderr);
}
