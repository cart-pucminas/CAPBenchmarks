#ifndef SLAVE_H_
#define SLAVE_H_

#define MATRIX_SEG_0 3
#define INFOS_SEG_0 4

/* Statistics to send back to IO */
typedef struct {
	size_t data_put;         /* Number of bytes put.    */
	size_t data_get;         /* Number of bytes gotten. */
	unsigned nput;           /* Number of put op.       */
	unsigned nget;	         /* Number of get op.      */
	uint64_t slave;          /* Time spent on slave.    */
	uint64_t communication;  /* Time spent on comms.    */
} Info;

/* Block element. */
struct element {
	int num; /* Numerator.   */
	int den; /* Denominator. */
}

/* Matrix block */
struct  {
	int width;                                      /* Block width.  */
	int height;                                     /* Block height. */
	element elements[CLUSTER_WORKLOAD/sizeof(element)]; /* Elements.     */
} block;

/* Pivot line. */
struct {
	int width;                                          /* Pivot line width. */ 
	element elements[CLUSTER_WORKLOAD/(4*sizeof(element))]; /* Elements.         */
} pvtline;

/* Returns the element [i][j] of the block. */
#define BLOCK(i, j) \
	(block.elements[block.width*(i) + (j)])

#endif