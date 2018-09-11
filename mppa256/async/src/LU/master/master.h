#ifndef MASTER_H_
#define MASTER_H_

extern void async_start();
extern void spawn_slave(int nCluster, char **args);
extern void join_slave(int nCluster);

/* Statistics results of slaves */
typedef struct {
	size_t data_sent;
	size_t data_received;
	unsigned nsent;
	unsigned nreceived;
	uint64_t slave;
	uint64_t communication;
} Info;

#endif