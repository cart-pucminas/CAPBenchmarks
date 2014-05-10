/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>.
 *
 * util/readline.c - readline() implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

/*
 * Reads a line from a file.
 */
char *readline(FILE *input)
{
	int n, length;
	char *s1, *s2, c;
	
	n = 0;
	length = 80;
	
	s1 = malloc((length + 1)*sizeof(char));

	/* Failed to allocate buffer. */
	if (s1 == NULL)
		goto out0;
	
	/* Read line. */
	while (((c = getc(input)) != eol) && (c != EOF))
	{
		/* Resize buffer. */
		if (n == length)
		{
			s2 = realloc(s1, length *= 2);
			
			/* Failed to allocate buffer. */
			if (s2 == NULL)
				goto out1;
				
			s1 = s2;
		}
		
		s1[n++] = c;
	}
	
	/* Nothing read. */
	if ((n == 0) && (c == EOF))
		goto out1;
	
	/* Extract line. */
	s1[n] = '\0';
	s2 = malloc((length + 1)*sizeof(char));
	strcpy(s2, s1);
	free(s1);
	
	return (s2);
	
out1:
	free(s1);
out0:
	return (NULL);	
}
