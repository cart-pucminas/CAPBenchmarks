#ifndef CLUSTER_H_
#define CLUSTER_H_

#ifdef _MASTER_
	extern void spawn_slaves();
	extern void join_slaves();
#endif

#endif