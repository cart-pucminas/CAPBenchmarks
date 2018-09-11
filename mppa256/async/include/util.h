#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>

/* Prints an error message and exits. */
extern void error(const char *msg);

/* Safe calloc(). */
extern void *scalloc(size_t nmemb, size_t size);

/* Safe malloc(). */
extern void *smalloc(size_t size);

/* Initializes the random number generator. */
extern void srandnum(int seed);

/* Generates a random number. */
extern unsigned randnum();

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

#else /* Slaves only functions */

/* Synchronization of all slaves */
void slave_barrier();

#endif /* _MASTER_ */

#endif /* UTIL_H_ */