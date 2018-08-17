#include <mppa_power.h>
#include <global.h>
#include <infos.h>
#include <spawn_util.h>
#include <stdio.h>

#ifdef _MASTER_

int spawn_slaves() {
	char arg0[4];   /* Argument 0. */
	char *args[1];  /* Arguments.  */
	for(int i = 0; i < nclusters; i++){
		sprintf(arg0, "%d", i);
		args[0] = arg0;
		if (mppa_power_base_spawn(i, "cluster_bin", (const char **)args , NULL, MPPA_POWER_SHUFFLING_ENABLED) == -1) {
			error_spawn(i);
		}
	}
	inform_clusters_started();
}

int join_slaves() {
	for(int i=0; i < nclusters; i++){
		int ret;
		if (mppa_power_base_waitpid(i, &ret, 0) < 0) {
			error_waitpid(i);
			return -1;
		}
	}
	return 0;
}

#endif