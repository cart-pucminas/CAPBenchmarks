/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * util/warning.c - warning() implementation.
 */

#include <stdio.h>
#include <stdarg.h>
#include <util.h>

/*
 * Prints a warning message in the standard error file.
 */
void warning(char *msg, ...)
{
	va_list args;
	char buffer[512];
	
	va_start(args, msg);
	vsprintf(buffer, msg, args);
	va_end(args);
	
	fprintf(stderr, "Warning: %s\n", buffer);
}
