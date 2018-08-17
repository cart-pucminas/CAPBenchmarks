/* Kernel Includes */
#include <global.h>
#include <problem.h>
#include <timer.h>

/* C And MPPA Library Includes*/
#include <stddef.h> 
#include <stdint.h> 
#include <mppa_async.h>
#include <utask.h>

void createSegment(mppa_async_segment_t*segment, unsigned long long ident, void *local, size_t size, unsigned flags, int multi, mppa_async_event_t *event) {
	mppa_async_segment_create(segment, ident, local, size, flags, multi, event);
}

void cloneSegment(mppa_async_segment_t *segment, unsigned long long ident, void *global, size_t size, mppa_async_event_t *event) {
	mppa_async_segment_clone(segment, ident, global, size, event);
}

void async_dataSend(void *item, mppa_async_segment_t *segment, int offset, int nItems, size_t type_size, mppa_async_event_t *event) {
	int start, end; /* Timing auxiliars */

	start = timer_get();

	mppa_async_put(item, segment, offset * type_size, type_size * nItems, event);

	end = timer_get();

	nsent += nItems;
	data_sent += nItems * type_size;
	communication += timer_diff(start, end);
}

void async_dataReceive(void *item,  mppa_async_segment_t *segment, int offset, int nItems, size_t type_size, mppa_async_event_t *event) {
	int start, end; /* Timing auxiliars */

	start = timer_get();

	mppa_async_get(item, segment, offset * type_size, type_size * nItems, event);

	end = timer_get();

	nreceived += nItems;
	data_received += nItems * type_size;
	communication += timer_diff(start, end);
}

void waitAllOpCompletion(mppa_async_segment_t *segment, mppa_async_event_t *event) {
	mppa_async_fence(segment, event);
}

void waitCondition(long long *local, long long value, mppa_async_cond_t cond, mppa_async_event_t *event) {
	mppa_async_evalcond(local, value, cond, event);
}

#ifdef _MASTER_

/* 	Initialization of async calling all necessary pre
	and pos functions 
*/
void async_master_start(){
	mppa_rpc_server_init(1, 0, nclusters);
	mppa_async_server_init();

	utask_t t;
	utask_create(&t, NULL, (void*)mppa_rpc_server_start, NULL);
}

void async_master_finalize() {
	mppa_async_server_final();
}

#else

void async_slave_init() {
	mppa_rpc_client_init();
	mppa_async_init();
}

void async_slave_finalize() {
	mppa_async_final();
	mppa_rpc_client_free();
}

#endif