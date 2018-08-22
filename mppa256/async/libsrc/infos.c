/* Kernel Includes */
#include <global.h>
#include <problem.h>
#include <timer.h>

/* C And MPPA Library Includes*/
#include <stdio.h>


#ifdef _MASTER_

void inform_clusters_started() {
	if (verbose) {
		printf("[IODDR0] Starts %d Cluster(s)\n", nclusters);
		fflush(stdout);
	}
}

void error_spawn(int idCluster) {
	if (verbose) {
		printf("#[IODDR0] Fail to Spawn cluster %d\n", idCluster);
		fflush(stdout);
	}

}

void error_waitpid(int idCluster) {
	printf("#[IODDR0] ERROR: Waitpid failed on cluster %d\n", idCluster);
	fflush(stdout);
}

void inform_usage() {
	printf("Usage: %s [options]\n", bench_initials);
	printf("Brief: %s Kernel\n", bench_fullName);
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
	fflush(stdout);
}

void inform_statistics() {
	printf("  total time: %f microseconds \n", total*MICROSEC);
	/*printf("timing statistics of %s (in microseconds):\n", initials);
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
	fflush(stdout);*/
}

void inform_actual_benchmark() {
	if (verbose) {
		printf(" Computing %s ... \n", bench_fullName);
		fflush(stdout);
	}
}

#endif