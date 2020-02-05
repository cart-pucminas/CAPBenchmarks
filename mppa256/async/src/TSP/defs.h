#ifndef __DEFS_H
#define __DEFS_H

#define MAX_TOWNS		21
#define MIN_JOBS_THREAD 50
#define MIN_PARTITIONS_PER_CLUSTER 20
#define MAX_MEM_PER_CLUSTER 1572864 //1.5MB

unsigned int get_number_of_partitions (int clusters);

#endif
