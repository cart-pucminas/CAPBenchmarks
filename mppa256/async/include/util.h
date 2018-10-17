#ifndef UTIL_H_
#define UTIL_H_
/* Kernel Includes */
#include <message.h>
#include <async_util.h>

/* C And MPPA Library Includes*/
#include <stddef.h>

/*************** COMMOM IO AND CC FUNCTIONS/VARIABLES ******************/

/* Signal segments ident. */
#define SIG_SEG_0 2

/* Signal offset exchange segment. */
extern mppa_async_segment_t signals_offset_seg;

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

/********************* MASTERS ONLY FUNCTIONS ************************/

#ifdef _MASTER_ /* Masters only functions */

/* Spawns CC with nCluster ID */
extern void spawn_slave(int nCluster, char **args);

/* Wait finalization of all CC */
extern void join_slaves();

/* Wait finalization of CC with nCluster ID */
extern void join_slave(int nCluster);

/********************* IO STATISTICS EXCHANGE ************************/

/* Waits for all slaves statistics. */
extern void wait_statistics();

/* Set slaves statistics. */
extern void set_statistics(struct message *information);

/********************* IO SYNCHRONIZATION ************************/

/* Signals context. */
extern off64_t sig_offsets[NUM_CLUSTERS];
extern long long cluster_signals[NUM_CLUSTERS];
extern char str_cc_signals_offset[NUM_CLUSTERS][50];

/* Get slave signal offset. */
extern void get_slaves_signals_offset();

/* Set signal offsets for the clusters. */
extern void set_cc_signals_offset();

/* Synchronization between slaves and IO. */
#define send_signal(i)                                          \
(postAdd(mppa_async_default_segment((i)), sig_offsets[(i)], 1)) \

/* Synchronization between slaves and IO. */
#define wait_signal(i)                                                    \
(mppa_async_evalcond(&cluster_signals[(i)], 1, MPPA_ASYNC_COND_EQ, NULL)) \

/********************* SLAVES ONLY FUNCTIONS ************************/

#else /* Slaves only functions */

/* Synchronization of all slaves */
extern void slave_barrier();

/********************* CC STATISTICS EXCHANGE ************************/

extern void send_statistics(mppa_async_segment_t *segment);

/********************* CC SYNCHRONIZATION SIGNAL ************************/

/* Signals exchange between IO and Clusters. */
extern long long io_signal;
extern off64_t sigback_offset;

/* Send slave signal offset to IO. */
extern void send_sig_offset();

/* Synchronization between slaves and IO. */
#define send_signal()                        \
postAdd(MPPA_ASYNC_DDR_0, sigback_offset, 1) \

/* Synchronization between slaves and IO. */
#define wait_signal()                                  \
waitCondition(&io_signal, 1, MPPA_ASYNC_COND_EQ, NULL) \

#endif /* _MASTER_ AND SLAVE */

#endif /* UTIL_H_ */