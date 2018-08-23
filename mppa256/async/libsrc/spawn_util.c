/* Kernel Includes */
#include <global.h>
#include <infos.h>
#include <spawn_util.h>

/* C And MPPA Library Includes*/
#include <stdio.h>
#include <stdlib.h>
#include <mppa_power.h>

#ifdef _MASTER_

void spawn_slave(int nCluster, char **args) {
	if (mppa_power_base_spawn(nCluster, "cluster_bin", (const char **)args, NULL, MPPA_POWER_SHUFFLING_ENABLED) == -1) {
		error_spawn(nCluster);
		exit(1);
	}
}

void join_slave(int nCluster) {
	int ret;
	if (mppa_power_base_waitpid(nCluster, &ret, 0) < 0) {
		error_waitpid(nCluster);
		exit(1);
	}
}

#endif