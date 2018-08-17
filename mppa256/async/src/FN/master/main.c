#include <infos.h>
#include <global.h>
#include <timer.h>
#include <stdio.h>

#include "master.h"

/* Timing statistics. */
uint64_t master = 0;          /* Time spent on master.        */
uint64_t slave[NUM_CLUSTERS]; /* Time spent on slaves.        */
uint64_t communication = 0;   /* Time spent on communication. */
uint64_t total = 0;           /* Total time.                  */

/* Data exchange statistics. */
size_t data_sent = 0;     /* Number of bytes received. */
unsigned nsend = 0;       /* Number of sends.          */
size_t data_received = 0; /* Number of bytes sent.     */
unsigned nreceive = 0;    /* Number of receives.       */

/* Problem. */
struct problem {
	int start; // Initial number of interval
	int end;   // Final number of invertal
};

static struct problem tiny     =  { 8000001, 8004096 };
static struct problem small    =  { 8000001, 8008192 };
static struct problem standard =  { 8000001, 8016384 };
static struct problem large    =  { 8000001, 8032768 };
static struct problem huge     =  { 8000001, 8065536 }; 

/* Benchmark parameters. */
int verbose = 0;   /* Display informations 	       */
int nclusters = 16; /* Quantity of Clusters Spawned */
static struct problem *p = &tiny;

int main(__attribute__((unused)) int argc, char **argv) {
	uint64_t startTime, endTime; /* Start and End Time. */
	int start_num = 0, end_num = 2;

	timer_init();

	startTime = timer_get();
	friendly_numbers(start_num, end_num);
	endTime = timer_get();
	total = timer_diff(startTime, endTime);

	printf("Executed in %.10f\n", total*MICROSEC);
	return 0;
}