#ifndef __COMMON_MAIN_H
#define __COMMON_MAIN_H

#include <timer.h>
#include "exec.h"

struct main_pars {
	int *nb_threads;
	int *nb_towns;
	int seed;
	int *nb_clusters;
	int nb_executions;	
	char *machine;
};

typedef union { 
	void *void_t;
	int int_t;
} barrier_par_t;

struct execution_parameters {
	int cluster;
	int nb_clusters;
	int nb_partitions;
	int nb_threads;
	int nb_towns;
	int seed;
	barrier_par_t barrier;
	tsp_t_pointer *tsp;
};

struct main_pars init_main_pars (int argc, char **argv);
void *spawn_worker (void* params);
void run_main (struct main_pars pars);
void free_main (struct main_pars pars);

extern pthread_t *spawn (tsp_t_pointer *tsp, int cluster_id, int nb_clusters, int nb_partitions, int nb_threads, int nb_towns, int seed, char* machine);
extern void run_tsp (int nb_threads, int nb_towns, int seed, int nb_clusters, char* machine);
extern void wait_barrier (barrier_par_t barrier_par);

#endif //__COMMON_MAIN_H
