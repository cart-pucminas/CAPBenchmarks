#ifndef MASTER_H_
#define MASTER_H_

extern void async_start();
extern void spawn_slave(int nCluster, char **args);
extern void join_slave(int nCluster);

void friendly_numbers(int start_num, int end_num);

#endif