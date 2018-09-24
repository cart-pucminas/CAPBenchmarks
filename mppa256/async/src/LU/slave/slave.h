#ifndef SLAVE_H_
#define SLAVE_H_

/* Statistics to send back to IO */
typedef struct {
	size_t data_put;         /* Number of bytes put.    */
	size_t data_get;         /* Number of bytes gotten. */
	unsigned nput;           /* Number of put op.       */
	unsigned nget;	         /* Number of get op.      */
	uint64_t slave;          /* Time spent on slave.    */
	uint64_t communication;  /* Time spent on comms.    */
} Info;

/* Matrix block */
struct  {
	int height;                                     /* Block height. */
	int width;                                      /* Block width.  */
	float elements[CLUSTER_WORKLOAD/sizeof(float)]; /* Elements.     */
} block;

/* Pivot line. */
struct {
	int width;                                          /* Pivot line width. */ 
	float elements[CLUSTER_WORKLOAD/(4*sizeof(float))]; /* Elements.         */
} pvtline;

/* Returns the element [i][j] of the block. */
#define BLOCK(i, j) \
	(block.elements[block.width*(i) + (j)])

#endif