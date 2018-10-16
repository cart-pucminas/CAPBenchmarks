#ifndef PROBLEM_H_
#define PROBLEM_H_

#ifdef _MASTER_

extern struct problem tiny;
extern struct problem small;
extern struct problem standard;
extern struct problem large;
extern struct problem huge;

extern struct problem *prob;
extern void readargs(int argc, char **argv);

#ifdef _FN_

struct problem {
	int start; // Initial number of interval
	int end;   // Final number of invertal
};

#elif _LU_

struct problem {
	int height; /* Matrix height. */
	int width;  /* Matrix width.  */
};

#endif /* Benchmarks */

#endif /* _MASTER_ */

#endif /* PROBLEM_H_ */