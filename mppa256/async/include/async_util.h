#ifndef ASYNC_UTIL_H_
#define ASYNC_UTIL_H_

/* Kernel Includes */
#include <mppa_async.h>

extern void createSegment(mppa_async_segment_t*segment, unsigned long long ident, void *local, size_t size, unsigned flags, int multi, mppa_async_event_t *event);
extern void cloneSegment(mppa_async_segment_t *segment, unsigned long long ident, void *global, size_t size, mppa_async_event_t *event);
extern void async_dataSend(void *item, mppa_async_segment_t *segment, int offset, int nItems, size_t type_size, mppa_async_event_t *event);
extern void async_dataReceive(void *item,  mppa_async_segment_t *segment, int offset, int nItems, size_t type_size, mppa_async_event_t *event);
extern void waitAllOpCompletion(mppa_async_segment_t *segment, mppa_async_event_t *event);
extern void waitCondition(long long *local, long long value, mppa_async_cond_t cond, mppa_async_event_t *event);

#ifdef _MASTER_
extern void async_master_start();
extern void async_master_finalize();
#else
/* don't do anything on slave side
   before calling this function    */
extern void async_slave_init();
extern void async_slave_finalize();
#endif

#endif