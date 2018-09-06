#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>

/* Prints an error message and exits. */
extern void error(const char *msg);

/* Safe calloc(). */
extern void *scalloc(size_t nmemb, size_t size);

/* Safe malloc(). */
extern void *smalloc(size_t size);

#ifdef _MASTER_ /* Masters only functions */

/* Spawns CC with nCluster ID */
extern void spawn_slave(int nCluster, char **args);

/* Wait finalization of CC with nCluster ID */
extern void join_slave(int nCluster);

/* Auxiliar progress strings */
extern char *bench_initials;
extern char *bench_fullName;

/* Auxiliar func. in case of args reading failure */
extern void inform_usage();

/* Show timing and data exchange statistics */
extern void inform_statistics();

/* Show what benchmark is executing */
extern void inform_actual_benchmark();

#else /* Slaves only functions */

/* Synchronization of all slaves */
void slave_barrier();

#endif

#endif