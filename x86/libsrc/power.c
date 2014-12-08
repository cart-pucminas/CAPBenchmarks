/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * power.c - Power utility library for Intel Xeon Phi
 */

#include <assert.h>
#include <stdio.h>
#include <pthread.h>

/*
 * Power information buffer. 
 */
struct
{
	unsigned tot0; /* Total power time window 0 (uW).                      */
	unsigned tot1; /* Total power time window 1 (uW).                      */
	unsigned pcie; /* Power measured at the PCI-express input.             */
	unsigned inst; /* Instantaneous power consumption reading (uW).        */
	unsigned imax; /* Maximum instantaneous power consumption observed.    */
	unsigned c2x3; /* Power measured at the 2x3 connector.                 */
	unsigned c2x4; /* Power measured at the 2x4 connector.                 */
	unsigned vccp; /* Power supply to the cores.                           */
	unsigned vddg; /* Power supply to everything but the cores and memory. */
	unsigned vddq; /* Power supply to the memory subsystem.                */
} power_buffer;

/*
 * /sys/class/micras/power
 */
static const char *micras_power_file = "/sys/class/micras/power";
static FILE *micras_power = NULL;


static const long time_stamp = 50000000L;
static pthread_t tid;
static double avg;
static unsigned live;

/*
 * Gets power information.
 */
static unsigned power_get(void)
{
	int error;
	
	fseek(micras_power, 0, SEEK_SET);
	
	error = fscanf(micras_power, "%*u %*u %*u %u", &power_buffer.inst);
	((void)error);
	
	return (power_buffer.inst);
}

/*
 * Listen to micras power file.
 */
static void *power_listen(void *unused)
{
	struct timespec ts;
	
	((void)unused);
	
	ts.tv_sec = 0;
	ts.tv_nsec = time_stamp;
	
	while (live)
	{
		avg = (avg + power_get())/2;
	
		nanosleep(&ts, NULL);
	}
	
	return (NULL);
}

/*
 * Initializes power measurement utility.
 */
void power_init(void)
{
	micras_power = fopen(micras_power_file, "r");
	assert(micras_power != NULL);
	
	live = 1;
	
	avg = power_get();
	
	pthread_create(&tid, NULL, power_listen, NULL);
}

/*
 * Terminates power measurement utility.
 */
double power_end(void)
{
	live = 0;
	pthread_join(tid, NULL);
	fclose(micras_power);
	
	return (avg);
}
