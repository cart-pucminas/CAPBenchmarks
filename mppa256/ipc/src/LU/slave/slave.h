#ifndef SLAVE_H_
#define SLAVE_H_

	/*
	 * Opens NoC connectors.
	 */
	extern void open_noc_connectors(void);
	
	/*
	 * Closes NoC connectors.
	 */
	extern void close_noc_connectors(void);

	extern int rank;    /* Process rank.         */
	extern int infd;    /* Input channel.        */
	extern int outfd;  /* Output channel.       */

	/* Matrix block */
	struct  {
		int width;                                      /* Block width.  */
		int height;                                     /* Block height. */
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

#endif /* SLAVE_H_ */
