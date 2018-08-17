#ifndef INFOS_H_
#define INFOS_H_

#ifdef _MASTER_
	extern void inform_clusters_started();
	extern void error_spawn(int idCluster);
	extern void error_waitpid(int idCluster);
	extern void usage(char *initials, char *benchName);
	extern void show_statics();
#endif

#endif