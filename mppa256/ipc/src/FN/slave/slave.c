/* 
 * File:   master.c - Mutually Friendly Numbers master process.
 * Authors: 
 *		Matheus A. Souza <matheusalcantarasouza@gmail.com>
 *      Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * Copyright(C) 2014
 *
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <mppaipc.h>
#include <stdint.h>
#include <stdlib.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>

/* Problem structure : number and abundances */
typedef struct {
    long int number; /* Number      */
    long int num; 	/* Numerator   */
    long int den; 	/* Denominator */
} Item;

#define MAX_TASK_SIZE 65536

/* Timing statistics. */
uint64_t start;
uint64_t end;
uint64_t communication = 0;
uint64_t total = 0;

/* Task sent by IO */
static Item task[MAX_TASK_SIZE];

/* Informations about the task */
static int tasksize;

/*
 * Computes the Greatest Common Divisor of two numbers.
 */
static int gcd(int a, int b) {
	int mod;

  /* Compute greatest common divisor. */
	while (b != 0)
	{
		mod = a % b;
		a = b;
		b = mod;
	}

	return a;
}

/*
 * Some of divisors. (Algorithm considering n >= 2)
 */
static int sumdiv(int n) {
	int sum;    /* Sum of divisors.     */
	int factor; /* Working factor.      */
	int maxD; 	/* Max divisor before n */

	maxD = (int)n/2;

	sum = (n == 1) ? 1 : 1 + n;
	
	/* Compute sum of divisors. */
	for (factor = 2; factor <= maxD; factor++) {
		/* Divisor found. */
		if ((n % factor) == 0)
			sum += factor;
	}
	return (sum);
}

static void calc_abundances() {
	int n; /* Divisor.      */
	int i; /* Loop indexes. */
	
	start = timer_get();

	/* Compute abundances. */
	for (i = 0; i < tasksize; i++) {		
		task[i].num = sumdiv(task[i].number);
		task[i].den = task[i].number;

		n = gcd(task[i].num, task[i].den);

		if (n != 0) {
			task[i].num /= n;
			task[i].den /= n;
		}
	}

	end = timer_get();

	/* Total slave time */
	total += timer_diff(start, end);
}


int main(__attribute__((unused)) int argc, char **argv) {
	/* Synchronization of timer */
	timer_init();
    
    /* Cluster id. */
    rank = atoi(argv[0]);
   
 	/* Get work from IO. */
    open_noc_connectors();
    data_receive(infd, &tasksize, sizeof(int));
	data_receive(infd, &task, tasksize*sizeof(Item));

	/* Calculation of all numbers abundances */
    calc_abundances();

	/* Send abundance and stats. to IO. */
    data_send(outfd, &task, tasksize*sizeof(Item));
	data_send(outfd, &total, sizeof(uint64_t));

    /* Close channels. */
    mppa_close(infd);
    mppa_close(outfd);
    mppa_exit(0);

    return (0);
}
