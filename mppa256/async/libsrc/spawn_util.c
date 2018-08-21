#include <mppa_power.h>
#include <global.h>
#include <infos.h>
#include <spawn_util.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _MASTER_

void spawn_slaves() {
	//char arg0[10];  /* Argument 0. */
	//char arg1[10];	/* Argument 1. */
	//char *args[2];  /* Arguments.  */

	//sprintf(arg0, "%d", start);
	//sprintf(arg1, "%d", start);
	//args[0] = arg0;
	//args[1] = arg1;
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