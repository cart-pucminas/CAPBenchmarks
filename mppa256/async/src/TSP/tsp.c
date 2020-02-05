/* Kernel Includes */
#include <arch.h>
#include "tsp.h"

/* C And MPPA Library Includes*/

int calc_max_hops (int nb_clusters, int nb_towns, int *max_hops_ret) {
    int total = 1;
    int max_hops = 0;
    while (total < MIN_JOBS_THREAD * NUM_THREADS * nb_clusters && max_hops < nb_towns - 1) {
		max_hops++;
		total *= nb_towns - max_hops;
	}
    max_hops++;
    if (max_hops_ret) *max_hops_ret = max_hops;
    return total;
}

int queue_size (int nb_clusters, int nb_towns, int *max_hops_ret) {
    int total = calc_max_hops (nb_clusters, nb_towns, max_hops_ret);
    int queue_size = total / nb_clusters + total % nb_clusters + nb_clusters - 1;
    return queue_size;
}