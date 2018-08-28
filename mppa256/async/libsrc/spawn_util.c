/* Kernel Includes */
#include <global.h>
#include <util.h>
#include <spawn_util.h>

/* C And MPPA Library Includes*/
#include <stdio.h>
#include <stdlib.h>
#include <mppa_power.h>

#ifdef _MASTER_

void spawn_slave(int nCluster, char **args) {
	if (mppa_power_base_spawn(nCluster, "cluster_bin", (const char **)args, NULL, MPPA_POWER_SHUFFLING_ENABLED) == -1)
		error("Error while spawning clusters\n");
}

void join_slave(int nCluster) {
	int ret;
	if (mppa_power_base_waitpid(nCluster, &ret, 0) < 0)
		error("Error while trying to join\n");
}

#endif