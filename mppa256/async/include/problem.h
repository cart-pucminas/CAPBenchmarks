#ifndef PROBLEM_H_
#define PROBLEM_H_

#ifdef _MASTER_

extern struct problem tiny;
extern struct problem small;
extern struct problem standard;
extern struct problem large;
extern struct problem huge;

extern struct problem *p;
extern void readargs(int argc, char **argv);

#ifdef _FN_

struct problem {
	int start; // Initial number of interval
	int end;   // Final number of invertal
};

//#elif

#endif /* Benchmarks */

#endif /* _MASTER_ */

#endif /* PROBLEM_H_ */