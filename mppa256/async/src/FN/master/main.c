/* Kernel Includes */
#include <util.h>
#include <global.h>
#include <timer.h>
#include <problem.h>
#include "master.h"

/* Problem initials and FullName */
char *bench_initials = "FN";
char *bench_fullName = "Friendly Numbers Benchmark Kernel";

/* Timing statistics. */
uint64_t master = 0;          /* Time spent on master.        */
uint64_t spawn = 0;           /* Time spent spawning slaves   */
uint64_t slave[NUM_CLUSTERS]; /* Time spent on slaves.        */
uint64_t communication = 0;   /* Time spent on communication. */
uint64_t total = 0;           /* Total time.                  */

/* Data exchange statistics. */
size_t data_put = 0;      /* Number of bytes put.    */
unsigned nput = 0;        /* Number of bytes gotten. */
size_t data_get = 0; /* Number of items put.    */
unsigned nget = 0;        /* Number of items gotten. */

/* Problem sizes */
struct problem tiny     =  { 8000001, 8004096 };
struct problem small    =  { 8000001, 8008192 };
struct problem standard =  { 8000001, 8016384 };
struct problem large    =  { 8000001, 8032768 };
struct problem huge     =  { 8000001, 8065536 }; 

/* Benchmark parameters. */
int verbose = 0;   /* Display informations? 	   */
int nclusters = 1; /* Quantity of Clusters spawned */
struct problem *prob = &tiny; /* Problem class */

/* Runs benchmark */
int main(int argc, const char **argv) {
	uint64_t startTime, endTime;

	readargs(argc, (char **) argv);

	timer_init();
	
	startTime = timer_get();
	friendly_numbers(prob->start, prob->end);
	endTime = timer_get();
	total = timer_diff(startTime, endTime);

	inform_statistics();
	
	return 0;
}