#ifndef _BARRIER_H_
#define _BARRIER_H_


	typedef struct{
	       int n;
	       int count;
	       sem_t mutex;
	       sem_t turnstyle;
	       sem_t turnstyle2;
	}barrier_t;


	extern void init_barrier(barrier_t *barrier, int n);

	extern void close_sem(barrier_t *barrier);

	extern void phase1_barrier(barrier_t *barrier);

	extern void phase2_barrier(barrier_t *barrier);

	extern void wait_barrier(barrier_t *barrier);


#endif
