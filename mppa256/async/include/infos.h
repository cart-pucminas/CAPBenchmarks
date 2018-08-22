#ifndef INFOS_H_
#define INFOS_H_

#ifdef _MASTER_
extern void inform_clusters_started();
extern void error_spawn(int idCluster);
extern void error_waitpid(int idCluster);
extern void inform_usage();
extern void inform_statistics();
extern void inform_actual_benchmark();
#endif

#endif