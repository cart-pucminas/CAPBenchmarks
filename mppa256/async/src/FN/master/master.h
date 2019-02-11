#ifndef MASTER_H_
#define MASTER_H_

/* Timing statistics and parcial sum result of slaves */
typedef struct {
	size_t data_put;
	size_t data_get;
	unsigned nput;
	unsigned nget;
	uint64_t slave;
	uint64_t communication;
	int parcial_sum;
} Info;

/* Items to be sent to slaves */
typedef struct {
    int number; /* Number      */
    int num; 	/* Numerator   */
    int den; 	/* Denominator */
} Item;

int friendly_numbers(int start_num, int end_num);

#endif