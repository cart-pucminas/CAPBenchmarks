#ifndef __JOB_H
#define __JOB_H

#include "defs.h"

typedef int path_t[MAX_TOWNS];

typedef struct {
	int len;
	path_t path;
} job_t;

typedef struct job_queue_node {
	job_t tsp_job;
	struct job_queue_node *next;
} job_queue_node_t;

#endif