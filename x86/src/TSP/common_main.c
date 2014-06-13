#define _GNU_SOURCE             

#include "common_main.h"

void *spawn_worker(void* params) {
	struct execution_parameters *p = (struct execution_parameters *)params;
	(*p->tsp) = init_execution(p->cluster, p->nb_clusters, p->nb_partitions, p->nb_threads, p->nb_towns, p->seed);
	
	wait_barrier (p->barrier);
	start_execution(*p->tsp);
	wait_barrier (p->barrier);
	end_execution(*p->tsp);

	free(params);
	return NULL;
}

struct main_pars init_main_pars (int argc, char **argv) {
	struct main_pars ret;
	
	CHECK_PAGE_SIZE();
	
	if (argc != 5 && argc != 6 && argc != 7) {
		fprintf (stderr, "Usage: %s <nb_threads> <nb_towns> <seed> <nb_clusters> [nb_executions machine]\n", argv[0]);
		exit(1);
	}

	ret.nb_threads = par_parse (argv[1]);
	assert(ret.nb_threads);
	ret.nb_towns = par_parse (argv[2]);
	assert(ret.nb_towns);
	ret.seed = atoi(argv[3]);
	ret.nb_clusters = par_parse (argv[4]);
	assert(ret.nb_clusters);
	ret.nb_executions = (argc == 6) ? atoi(argv[5]) : 1;
	assert(ret.nb_executions > 0);
	ret.machine = (argc == 7) ? argv[6] : NULL;

	init_time();

	return ret;
}

void run_main (struct main_pars pars) {
	int execution, town, cluster, thread;
	town = 0;
	while (pars.nb_towns[town] != 0) {
		cluster = 0;
		while (pars.nb_clusters[cluster] != 0) {
			thread = 0;
			while (pars.nb_threads[thread] != 0) {
				assert (pars.nb_threads[thread] > 0);
				assert (pars.nb_towns[town] <= MAX_TOWNS);
				assert (pars.nb_clusters[cluster] > 0);
				for (execution = 0; execution < pars.nb_executions; execution++)
					run_tsp(pars.nb_threads[thread], pars.nb_towns[town], pars.seed, pars.nb_clusters[cluster], pars.machine);
				thread++;
			}
			cluster++;
		}
		town++;
	}
}

void free_main (struct main_pars pars) {
	free(pars.nb_towns);
	free(pars.nb_threads);
	free(pars.nb_clusters);
}