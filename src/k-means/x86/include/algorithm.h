/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmai.com>
 * 
 * <algorithm.h> - Algorithms library.
 */

#ifndef ALGORITHM_H_
#define ALGORITHM_H_

	#include <array.h>
	
	/*
	 *  Clusters data.
	 */
	int *kmeans(array_t data, int nclusters, float mindistance);

#endif /* ALGORITHM_H_ */
