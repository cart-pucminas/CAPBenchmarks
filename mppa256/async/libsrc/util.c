/* Kernel Includes */
#include <global.h>
#include <timer.h>
#include <util.h>
#include <message.h>

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

void set_statistics(struct message *information) {
	uint64_t comm_Sum = 0;
	uint64_t comm_Average = 0;
	for (int i = 0; i < nclusters; i++) {
		slave[i] = information[i].u.info.total;
		comm_Sum += information[i].u.info.communication;
		data_put += information[i].u.info.data_put;
		data_get += information[i].u.info.data_get;
		nput += information[i].u.info.nput;
		nget += information[i].u.info.nget;
	}

	comm_Average = (uint64_t)(comm_Sum+communication)/(nclusters+1);
	communication = comm_Average;
}

#else

void send_statistics(mppa_async_segment_t *segment, off64_t offset) {
	struct message *msg = message_create(STATISTICSINFO, data_put, data_get, nput, nget, total, communication);

	/* Puts message with statistics infos. in IO msg remote seg. */
	message_put(msg, segment, cid, NULL);

	/* Send stats. ready signal to IO. */
	postAdd(MPPA_ASYNC_DDR_0, offset, 1);

	message_destroy(msg);
}

/* Synchronization of all slaves */
void slave_barrier() {
	uint64_t start, end; /* Timing auxiliars */

	start = timer_get();
	mppa_rpc_barrier_all();
	end = timer_get();

	communication += timer_diff(start, end);
}

#endif