/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * Integer Sort Benchmark for the x86 architecture.
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "integer-sort.h"

/* Benchmark parameters. */
int nthreads;
int problemsize;

/*
 * Integer Sort benchmark.
 */
int main(int argc, char **argv)
{
	int i;
	int *a;
	
	problemsize = atoi(argv[1]);
	nthreads = atoi(argv[2]);

	srand(0);
	omp_set_num_threads(nthreads);
	
	a = malloc(problemsize*sizeof(int));
	
	for (i = 0; i < problemsize; i++)
		a[i] = rand() & 0xfffff;

	bucketsort(a, problemsize);

	/* Housekeeping. */
	free(a);
	
	return (0);
}
