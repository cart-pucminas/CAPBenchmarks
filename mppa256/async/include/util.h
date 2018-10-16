#ifndef UTIL_H_
#define UTIL_H_
/* Kernel Includes */
#include <message.h>
#include <async_util.h>

/* C And MPPA Library Includes*/
#include <stddef.h>

/* Prints an error message and exits. */
extern void error(const char *msg);

/* Prints an warning message */
extern void warning(const char *msg);

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

/* Wait finalization of all CC */
extern void join_slaves();

/* Wait finalization of CC with nCluster ID */
extern void join_slave(int nCluster);

extern void set_statistics(struct message *information);

#else /* Slaves only functions */

extern void send_statistics(mppa_async_segment_t *segment, off64_t offset);

/* Synchronization of all slaves */
extern void slave_barrier();

#endif /* _MASTER_ */

#endif /* UTIL_H_ */