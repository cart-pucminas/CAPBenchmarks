/* Kernel Includes */
#include <global.h>
#include <timer.h>
#include <util.h>

/* C And MPPA Library Includes*/
#include <mppa_power.h>
#include <mppa_rpc.h>
#include <stdlib.h>
#include <stdio.h>

/* Prints an error message and exits. */
void error(const char *msg) {
	fprintf(stderr, "error: %s\n", msg);
	exit(EXIT_FAILURE);
}

/* Prints a warning message. */
void warning(const char *msg) {
	fprintf(stderr, "warning: %s\n", msg);
}

/* Safe calloc(). */
void *scalloc(size_t nmemb, size_t size) {
	void *p;
	
	p = calloc(nmemb, size);
	
	/* Failed to allocate memory. */
	if (p == NULL)
		error("cannot calloc()");
	
	return (p);
}

/* Safe malloc(). */
void *smalloc(size_t size) {
	void *p;
	
	p = malloc(size);
	
	/* Failed to allocate memory. */
	if (p == NULL)
		error("cannot malloc()");
	
	return (p);
}

#define RANDNUM_W 521288629;
#define RANDNUM_Z 362436069;

static unsigned randum_w = RANDNUM_W;
static unsigned randum_z = RANDNUM_Z;

/* Initializes the random number generator. */
void srandnum(int seed) {
	unsigned w, z;

	w = (seed * 104623) & 0xffffffff;
	randum_w = (w) ? w : RANDNUM_W;
	z = (seed * 48947) & 0xffffffff;
	randum_z = (z) ? z : RANDNUM_Z;
}

/* Generates a random number. */
unsigned randnum() {
	unsigned u;
	
	/* 0 <= u < 2^32 */
	randum_z = 36969 * (randum_z & 65535) + (randum_z >> 16);
	randum_w = 18000 * (randum_w & 65535) + (randum_w >> 16);
	u = (randum_z << 16) + randum_w;
	
	return u;
}

#ifdef _MASTER_

/* Spawns CC with nCluster ID */
void spawn_slave(int nCluster, char **args) {
	if (mppa_power_base_spawn(nCluster, "cluster_bin", (const char **)args, NULL, MPPA_POWER_SHUFFLING_ENABLED) == -1)
		error("Error while spawning clusters\n");
}

/* Wait finalization of all CC */
void join_slaves() {
	#pragma omp parallel for default(shared) num_threads(3)
	for (int i = 0; i < nclusters; i++)
		join_slave(i);
}

/* Wait finalization of CC with nCluster ID */
void join_slave(int nCluster) {
	int ret;
	if (mppa_power_base_waitpid(nCluster, &ret, 0) < 0)
		error("Error while trying to join\n");
}

/* Auxiliar func. in case of args reading failure */
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

/* Show timing and data exchange statistics */
void inform_statistics() {
	printf("timing statistics of %s:\n", bench_initials);
	printf("  master:        %f\n", master*MICROSEC);
	for (int i = 0; i < nclusters; i++)
		printf("  slave %d:      %f\n", i, slave[i]*MICROSEC);
	printf("  spawn %d CC:   %f\n", nclusters, spawn*MICROSEC);
	printf("  communication: %f\n", communication*MICROSEC);
	printf("  total time:    %f\n", total*MICROSEC);
	printf("asynchronous operations statistics:\n");
	printf("  data put:         %d\n", data_put);
	printf("  data got:         %d\n", data_get);
	printf("  number of puts:   %u\n", nput);
	printf("  number of gets:   %u\n", nget);
	fflush(stdout);
}

#else

/* Synchronization of all slaves */
void slave_barrier() {
	mppa_rpc_barrier_all();
}

#endif