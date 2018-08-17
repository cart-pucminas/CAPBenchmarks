#include <stdio.h>
#include <global.h>
#include <infos.h>
#include <timer.h>

#ifdef _MASTER_

void inform_clusters_started() {
	if (verbose)
		printf("[IODDR0] Starts %d Cluster(s)\n", nclusters);
}

void error_spawn(int idCluster) {
	if (verbose)
		printf("#[IODDR0] Fail to Spawn cluster %d\n", idCluster);
}

void error_waitpid(int idCluster) {
	printf("#[IODDR0] ERROR: Waitpid failed on cluster %d\n", idCluster);
}

void usage(char *initials, char *benchName) {
	printf("Usage: %s [options]\n", initials);
	printf("Brief: %s Kernel\n", benchName);
	printf("Options:\n");
	printf("  --help             Display this information and exit\n");
	printf("  --nclusters <value> Set number of threads\n");
	printf("  --class <name>     Set problem class:\n");
	printf("                       - tiny\n");
	printf("                       - small\n");
	printf("                       - standard\n");
	printf("                       - large\n");
	printf("                       - huge\n");
	printf("  --verbose          Be verbose\n");
}

void show_statics() {
	printf("timing statistics:\n");
	printf("  master:        %f\n", master*MICROSEC);
	for (int i = 0; i < nclusters; i++)
		printf("  slave %d:      %f\n", i, slave[i]*MICROSEC);
	printf("  communication: %f\n", communication*MICROSEC);
	printf("  total time:    %f\n", total*MICROSEC);
	printf("data exchange statistics:\n");
	printf("  data sent:            %d\n", data_sent);
	printf("  number sends:         %u\n", nsend);
	printf("  data received:        %d\n", data_received);
	printf("  number receives:      %u\n", nreceive);
}


#endif