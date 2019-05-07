#ifndef PROBLEM_H_
#define PROBLEM_H_

#ifdef _MASTER_

/* Auxiliar progress strings */
extern char *bench_initials;
extern char *bench_fullName;

extern struct problem tiny;
extern struct problem small;
extern struct problem standard;
extern struct problem large;
extern struct problem huge;

extern struct problem *prob;
extern void readargs(int argc, char **argv);

/* Auxiliar func. in case of args reading failure */
extern void inform_usage();

/* Show timing and data exchange statistics */
extern void inform_statistics();

#ifdef _FN_

struct problem {
	int start; /* Initial number of interval. */
	int end;   /* Final number of invertal.   */
};

#elif _LU_

struct problem {
	int height; /* Matrix height. */
	int width;  /* Matrix width.  */
};

#elif _KM_

struct problem {
	int npoints;    /* Number of points.    */
	int ncentroids; /* Number of centroids. */
};

#elif _GF_

struct problem {
	int masksize; /* mask size.  */
	int imgsize;  /* Image size. */
};

#elif _FAST_

struct problem{
	int maskcolumns; /* Mask columns. */
	int maskrows;    /* Mask Rows.    */
	int masksize;	 /* Mask Size.    */
	int imgsize;  	 /* Image size.   */
};

#elif _IS_

struct problem {
	int n; /* Number of elements. */
};

#endif /* Benchmarks */

#endif /* _MASTER_ */

#endif /* PROBLEM_H_ */