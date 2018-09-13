#ifndef ASYNC_UTIL_H_
#define ASYNC_UTIL_H_

/* Kernel Includes */
#include <mppa_async.h>

/*
 * Obs: You can delay the waiting of all wait functions
 * using the event param. It's present in all wait like 
 * functions. In This case, the final wait will be when 
 * you call some of the async_wait_event... functions
 */

/* Put data on remote segment */
extern void dataPut(void *item, mppa_async_segment_t *segment, int offset, int nItems, size_t type_size, mppa_async_event_t *event);

/* Get data from remote segment */
extern void dataGet(void *item,  mppa_async_segment_t *segment, int offset, int nItems, size_t type_size, mppa_async_event_t *event);

/* Waits all PUT/GET operations of some seg. to complete. */
extern void waitAllOpCompletion(mppa_async_segment_t *segment, mppa_async_event_t *event);

/* Waits for some condition to occur (Use event to wait later) */
extern void waitCondition(long long *local, long long value, mppa_async_cond_t cond, mppa_async_event_t *event);

#ifdef _MASTER_

/* Initializes a unique segment */
extern void createSegment(mppa_async_segment_t*segment, unsigned long long ident, void *local, size_t size, unsigned flags, int multi, mppa_async_event_t *event);

/* Necessary func. calls to initialize async context */
extern void async_master_start();

/* Finalizes async context on master*/
extern void async_master_finalize();

#else

/* Initalizes async context on slave */
extern void cloneSegment(mppa_async_segment_t *segment, unsigned long long ident, void *global, size_t size, mppa_async_event_t *event);

/* Be aware of some unique segment */
extern void async_slave_init();

/* Finalizes async context on slave */
extern void async_slave_finalize();

#endif

#endif