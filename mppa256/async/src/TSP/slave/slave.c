/* Kernel Includes */

/* C And MPPA Library Includes*/
#include <stdint.h>
#include <stdlib.h>

/* Individual Slave statistics. */
uint64_t total = 0;          /* Time spent on slave.    */
uint64_t communication = 0;  /* Time spent on comms.    */               
size_t data_put = 0;         /* Number of bytes put.    */
size_t data_get = 0;         /* Number of bytes gotten. */
unsigned nput = 0;           /* Number of put ops.      */
unsigned nget = 0;           /* Number of get ops.      */

/* Compute Cluster ID & num. of clusters being used. */
int cid;

int main(__attribute__((unused))int argc, char **argv) {

	return 0;
}