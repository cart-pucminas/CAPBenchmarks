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
#include <omp.h>
#include <mppa/osconfig.h>
#include "mfn.h"
#include "defines.h"

//#define NB_THREADS 16

/* Inter process communication. */
static int rank; /* Process rank.   */
static int channel_in_fd; /* Input channel.  */
static int channel_out_fd; /* Output channel. */

static Item task[65536];

static void syncNumbers(long int TASK_SIZE) {
    ssize_t n, nOfN;
    ssize_t count;
	nOfN = sizeof (long int);
	
    n = TASK_SIZE * sizeof (Item);
    count = mppa_write(channel_out_fd, &n, nOfN);
    assert(count != -1);
    count = mppa_write(channel_out_fd, &task, n);
    assert(count != -1);
}

static void getwork() {
    ssize_t n;
    ssize_t count;

    count = mppa_read(channel_in_fd, &n, sizeof(ssize_t));
    assert(count != -1);
	//printf("Size n: %d - Rank: %d\n", n, rank);
	count = mppa_read(channel_in_fd, &task, n);
    assert(count != -1);
	//printf("Slave %d got %d bytes of work!\n",rank, n);
}

static int gcd_iter(int u, int v) {
	int t;
	while(v) {
		t=u;
		u=v;
		v=t%v;
	}
	return u<0 ? -u : u;
}

static void friendlyNumbers(long int TASK_SIZE) {
#pragma omp parallel num_threads(NB_THREADS)
    {
        long int i, factor, sum, done, n;

#pragma omp for schedule (static)
        for (i = 0; i < TASK_SIZE; i++) {
            long int number = task[i].number;
            sum = 1 + number;
            done = number;
            factor = 2;
            while (factor < done) {
                if ((number % factor) == 0) {
                    sum += (factor + (number / factor));
                    if ((done = number / factor) == factor)
                        sum -= factor;
                }
                factor++;
            }
            task[i].numerator = sum;
            task[i].denominator = number;
            n = gcd_iter(task[i].numerator, task[i].denominator);
            task[i].numerator /= n;
            task[i].denominator /= n;
        }
    }
}

int main(int argc, char **argv) {
    char path[35];
    uint64_t mask; /* Mask for sync.        */
    int sync_fd; /* Sync file descriptor. */
	long int TASK_SIZE;

    ((void) argc);

    rank = atoi(argv[0]);
	TASK_SIZE = atoi(argv[1]);

    sync_fd = mppa_open("/mppa/sync/128:64", O_WRONLY);
    assert(sync_fd != -1);

    /* Open channels. */
    sprintf(path, "/mppa/channel/%d:%d/128:%d", rank, rank + 1, rank + 1);
    channel_in_fd = mppa_open(path, O_RDONLY);
    assert(channel_in_fd != -1);
    sprintf(path, "/mppa/channel/128:%d/%d:%d", rank + 33, rank, rank + 33);
    channel_out_fd = mppa_open(path, O_WRONLY);
    assert(channel_out_fd != -1);
    
    // Synchronize with master.
    mask = 1 << rank;
    assert(mppa_write(sync_fd, &mask, 8) == 8);
	mppa_close(sync_fd);
	
    getwork();

    friendlyNumbers(TASK_SIZE);

    syncNumbers(TASK_SIZE);

    /* Close channels. */
    mppa_close(channel_in_fd);
    mppa_close(channel_out_fd);

    mppa_exit(0);
    return (0);
}
