#ifndef CLUSTER_H_
#define CLUSTER_H_

#ifdef _MASTER_
extern void spawn_slave(int nCluster, char **args);
extern void join_slave(int nCluster);
#endif

#endif