#include <stddef.h> // Necessary for size_t.
#include <stdint.h> // Necessary for uint64_t.
#include <mppa_async.h>
#include <utask.h>
#include <global.h>
#include <infos.h>

#ifdef _MASTER_

/* 	Initialization of async calling all necessary pre
	and pos functions 
*/
void async_master_init(){
	mppa_rpc_server_init(1, 0, nclusters);
	mppa_async_server_init();
}

void async_master_start() {
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