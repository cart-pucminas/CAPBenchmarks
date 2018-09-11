/* Kernel Include */
#include <async_util.h>
#include <global.h>

/* Individual Slave statistics. */
uint64_t total = 0;         /* Time spent on slave.    */
uint64_t communication = 0; /* Time spent on comms.    */               
size_t data_sent = 0;       /* Number of bytes put.    */
size_t data_received = 0;   /* Number of bytes gotten. */
unsigned nsent = 0;         /* Number of items put.    */
unsigned nreceived = 0;     /* Number of items gotten. */

/* Statistics to send back to IO */
typedef struct {
	size_t data_sent;
	size_t data_received;
	unsigned nsent;
	unsigned nreceived;
	uint64_t slave;
	uint64_t communication;
} Info;

int main(__attribute__((unused))int argc, char **argv) {

	return 0;
}