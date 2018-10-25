/* Kernel Includes */
#include <global.h>
#include <problem.h>
#include <timer.h>
#include <util.h>

/* C And MPPA Library Includes*/
#include <stddef.h> 
#include <stdint.h> 
#include <mppa_async.h>
#include <utask.h>

/*============================================================================*
 *                   COMMOM IO AND CC FUNCTIONS/VARIABLES                     *
 *============================================================================*/

static uint64_t start, end; /* Timing auxiliars */

/* Put data on remote segment. */
void dataPut(void *item, mppa_async_segment_t *segment, int offset, int nItems, size_t type_size, mppa_async_event_t *event) {
	start = timer_get();
	mppa_async_put(item, segment, offset * type_size, type_size * nItems, event);
	end = timer_get();

	nput++;
	data_put += nItems * type_size;
	communication += timer_diff(start, end);
}

/* Put spaced data on remote segment. */
void dataPutSpaced(const void *local, const mppa_async_segment_t *segment, off64_t offset, size_t size, int count, size_t space, mppa_async_event_t *event) {
	start = timer_get();
	mppa_async_put_spaced(local, segment, offset, size, count, space, event);
	end = timer_get();

	nput++;
	data_put += count * size;
	communication += timer_diff(start, end);
}

/* Get data from remote segment */
void dataGet(void *item,  mppa_async_segment_t *segment, int offset, int nItems, size_t type_size, mppa_async_event_t *event) {
	start = timer_get();
	mppa_async_get(item, segment, offset * type_size, type_size * nItems, event);
	end = timer_get();

	nget++;
	data_get += nItems * type_size;
	communication += timer_diff(start, end);
}

/* Get spaced data from remote segment. */
void dataGetSpaced(void *local, const mppa_async_segment_t *segment, off64_t offset, size_t size, int count, size_t space, mppa_async_event_t *event) {
	start = timer_get();
	mppa_async_get_spaced(local, segment, offset, size , count, space, event);
	end = timer_get();

	nget++;
	data_get += count * size;
	communication += timer_diff(start, end);
}

/* Waits all PUT/GET operations of some seg. to complete. */
void waitAllOpCompletion(mppa_async_segment_t *segment, mppa_async_event_t *event) {
	start = timer_get();
	mppa_async_fence(segment, event);
	end = timer_get();
	communication += timer_diff(start, end);
}

/* Waits for some condition to occur */
void waitCondition(long long *local, long long value, mppa_async_cond_t cond, mppa_async_event_t *event) {
	start = timer_get();
	mppa_async_evalcond(local, value, cond, event);
	end = timer_get();
	communication += timer_diff(start, end);
}

/* Waits an event to complete. */
void waitEvent(mppa_async_event_t *event) {
	start = timer_get();
	mppa_async_event_wait(event);
	end = timer_get();
	communication += timer_diff(start, end);
}

/* Post a poke remote long long datum. */
void poke(const mppa_async_segment_t *segment, off64_t offset, long long value) {
	start = timer_get();
	mppa_async_poke(segment, offset, value);
	end = timer_get();

	communication += timer_diff(start, end);
}

/*============================================================================*
 *                          MASTERS ONLY FUNCTIONS/VARIABLES                  *
 *============================================================================*/

#ifdef _MASTER_

/* Initializes a unique segment */
void createSegment(mppa_async_segment_t *segment, unsigned long long ident, void *local, size_t size, unsigned flags, int multi, mppa_async_event_t *event) {
	mppa_async_segment_create(segment, ident, local, size, flags, multi, event);
}

/* Necessary func. calls to initialize async context */
void async_master_start() {
	mppa_rpc_server_init(1, 0, nclusters);
	mppa_async_server_init();

	utask_t t;
	utask_create(&t, NULL, (void*)mppa_rpc_server_start, NULL);
}

/* Finalizes async context on master*/
void async_master_finalize() {
	mppa_async_server_final();
	mppa_rpc_server_free();
}

/*============================================================================*
 *                          SLAVES ONLY FUNCTIONS/VARIABLES                   *
 *============================================================================*/

#else

/* Initalizes async context on slave */
void async_slave_init() {
	mppa_rpc_client_init();
	mppa_async_init();
}

/* Be aware of some unique segment */
void cloneSegment(mppa_async_segment_t *segment, unsigned long long ident, void *global, size_t size, mppa_async_event_t *event) {
	mppa_async_segment_clone(segment, ident, global, size, event);
}

/* Finalizes async context on slave */
void async_slave_finalize() {
	mppa_async_final();
	mppa_rpc_client_free();
}

#endif