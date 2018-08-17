#ifndef CLUSTER_H_
#define CLUSTER_H_

#ifdef _MASTER_
	extern int spawn_slaves();
	extern int join_slaves();
#endif

#endif