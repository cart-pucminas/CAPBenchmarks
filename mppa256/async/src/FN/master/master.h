#ifndef MASTER_H_
#define MASTER_H_

extern void async_start();
extern void spawn_slave(int nCluster, char **args);
extern void join_slave(int nCluster);

/* Timing statistics and parcial sum result */
typedef struct {
	size_t data_sent;
	size_t data_received;
	unsigned nsent;
	unsigned nreceived;
	uint64_t slave;
	uint64_t communication;
	int parcial_sum;
} Info;

typedef struct {
    int number; /* Number      */
    int num; 	/* Numerator   */
    int den; 	/* Denominator */
} Item;

int friendly_numbers(int start_num, int end_num);

#endif