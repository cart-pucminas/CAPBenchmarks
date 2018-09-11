/* Kernel Includes */
#include <util.h>
#include <problem.h>
#include <global.h>

/* C And MPPA Library Includes*/
#include <string.h>
#include <stdlib.h>

#ifdef _MASTER_

void readargs(int argc, char **argv) {
	int i;     /* Loop index.       */
	char *arg; /* Working argument. */
	int state; /* Processing state. */
	
	/* State values. */
	#define READ_ARG     0 /* Read argument.         */
	#define SET_nclusters 1 /* Set number of clusters. */
	#define SET_CLASS    2 /* Set problem class.     */
	
	state = READ_ARG;
	
	/* Read command line arguments. */
	for (i = 1; i < argc; i++)
	{
		arg = argv[i];
		
		/* Set value. */
		if (state != READ_ARG)
		{
			switch (state)
			{
				/* Set problem class. */
				case SET_CLASS :
					if (!strcmp(argv[i], "tiny"))
						prob = &tiny;
					else if (!strcmp(argv[i], "small"))
						prob = &small;
					else if (!strcmp(argv[i], "standard"))
						prob = &standard;
					else if (!strcmp(argv[i], "large"))
						prob = &large;
					else if (!strcmp(argv[i], "huge"))
						prob = &huge;
					else 
						inform_usage();
					state = READ_ARG;
					break;
				
				/* Set number of threads. */
				case SET_nclusters :
					nclusters = atoi(arg);
					state = READ_ARG;
					break;

				default:
					inform_usage();		
			}
			continue;
		}
		
		/* Parse argument. */
		if (!strcmp(arg, "--verbose"))
			verbose = 1;
		else if (!strcmp(arg, "--nclusters"))
			state = SET_nclusters;
		else if (!strcmp(arg, "--class"))
			state = SET_CLASS;
		else
			inform_usage();
	}
	
	/* Invalid argument(s). */
	if (nclusters < 1)
		inform_usage();
}

#endif