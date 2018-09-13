/* Kernel Include */
#include <async_util.h>
#include <global.h>
#include <timer.h>
#include "slave.h"

/* C And MPPA Library Includes*/
#include <stdio.h>

/* Individual Slave statistics. */
uint64_t total = 0;          /* Time spent on slave.    */
uint64_t communication = 0;  /* Time spent on comms.    */               
size_t data_put = 0;         /* Number of bytes put.    */
size_t data_get = 0;         /* Number of bytes gotten. */
unsigned nput = 0;           /* Number of put ops.      */
unsigned nget = 0;           /* Number of get ops.      */

int main(__attribute__((unused))int argc,__attribute__((unused)) char **argv) {
	/* Initializes async client */
	async_slave_init();

	/* Problem information */
	int i0, j0;          /* Block start.             */
	int ipvt;            /* ith idex of pivot.       */
	int jpvt;            /* jth index of pivot.      */
	size_t n;            /* Number of bytes to send. */
	struct message *msg; /* Message.                 */

	/* Timer synchronization */
	timer_init();

	/* Cluster ID */
	cid = __k1_get_cluster_id();

	/* Finalizes async library and rpc client */
	async_slave_finalize();

	return 0;
}