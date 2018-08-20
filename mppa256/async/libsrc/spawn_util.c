#include <mppa_power.h>
#include <global.h>
#include <infos.h>
#include <spawn_util.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _MASTER_

void spawn_slaves() {
	for(int i = 0; i < nclusters; i++){
		if (mppa_power_base_spawn(i, "cluster_bin", (const char **)NULL , NULL, MPPA_POWER_SHUFFLING_ENABLED) == -1) {
			error_spawn(i);
			exit(1);
		}
	}
	inform_clusters_started();
}

void join_slaves() {
	for(int i=0; i < nclusters; i++){
		int ret;
		if (mppa_power_base_waitpid(i, &ret, 0) < 0) {
			error_waitpid(i);
			exit(1);
		}
	}
}

#endif