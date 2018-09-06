/* Kernel Includes */
#include <global.h>
#include <timer.h>
#include <util.h>

/* C And MPPA Library Includes*/
#include <mppa_power.h>
#include <mppa_rpc.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * Prints an error message and exits.
 */
void error(const char *msg)
{
	fprintf(stderr, "error: %s\n", msg);
	exit(EXIT_FAILURE);
}

/*
 * Safe calloc().
 */
void *scalloc(size_t nmemb, size_t size)
{
	void *p;
	
	p = calloc(nmemb, size);
	
	/* Failed to allocate memory. */
	if (p == NULL)
		error("cannot calloc()");
	
	return (p);
}

/*
 * Safe malloc().
 */
void *smalloc(size_t size)
{
	void *p;
	
	p = malloc(size);
	
	/* Failed to allocate memory. */
	if (p == NULL)
		error("cannot malloc()");
	
	return (p);
}

#ifdef _MASTER_

void spawn_slave(int nCluster, char **args) {
	if (mppa_power_base_spawn(nCluster, "cluster_bin", (const char **)args, NULL, MPPA_POWER_SHUFFLING_ENABLED) == -1)
		error("Error while spawning clusters\n");
}

void join_slave(int nCluster) {
	int ret;
	if (mppa_power_base_waitpid(nCluster, &ret, 0) < 0)
		error("Error while trying to join\n");
}

void inform_usage() {
	printf("Usage: %s [options]\n", bench_initials);
	printf("Brief: %s Kernel\n", bench_fullName);
	printf("Options:\n");
	printf("  --help             Display this information and exit\n");
	printf("  --nclusters <value> Set number of threads\n");
	printf("  --class <name>     Set problem class:\n");
	printf("                       - tiny\n");
	printf("                       - small\n");
	printf("                       - standard\n");
	printf("                       - large\n");
	printf("                       - huge\n");
	printf("  --verbose          Be verbose\n");
	fflush(stdout);
}

void inform_statistics() {
	printf("timing statistics of %s:\n", bench_initials);
	printf("  master:        %f\n", master*MICROSEC);
	for (int i = 0; i < nclusters; i++)
		printf("  slave %d:      %f\n", i, slave[i]*MICROSEC);
	printf("  spawn %d CC:   %f\n", nclusters, spawn*MICROSEC);
	printf("  communication: %f\n", communication*MICROSEC);
	printf("  total time:    %f\n", total*MICROSEC);
	printf("data exchange statistics:\n");
	printf("  data sent:            %d\n", data_sent);
	printf("  number sends:         %u\n", nsent);
	printf("  data received:        %d\n", data_received);
	printf("  number receives:      %u\n", nreceived);
	fflush(stdout);
}

void inform_actual_benchmark() {
	if (verbose) {
		printf(" Computing %s ... \n", bench_fullName);
		fflush(stdout);
	}
}

#else

void slave_barrier() {
	mppa_rpc_barrier_all();
}

#endif