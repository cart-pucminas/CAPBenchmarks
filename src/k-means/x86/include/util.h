/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <util.h> - Utility library.
 */

#ifndef UTIL_H_
#define UTIL_H_

	#include <stdarg.h>
	#include <stdio.h>
	
	/*
	 * Prints a debug message in the standard error file .
	 */
	extern void debug(char *msg, ...);
	
	/*
	 * Prints an error message in the standard error file and exits.
	 */
	extern void error(char *msg, ...);

	/*
	 * Reads a line from a file.
	 */
	extern char *readline(FILE *input);
	
	/*
	 * Set end of line character.
	 */
	extern char seteol(char c);
	
	/*
	 * Prints a warning message in the standard error file.
	 */
	extern void warning(char *msg, ...);

	/*
	 * Unused paramter.
	 */
	#define UNUSED(x) ((void)x)
	
	/*
	 * Returns the size of an array.
	 */
	#define SIZEOF_ARRAY(x) (sizeof((x))/sizeof((x)[0]))

#endif /* UTIL_H_ */
