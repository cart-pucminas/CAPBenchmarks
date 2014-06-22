/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * gf.h - Gaussian Filter kernel library.
 */

#ifndef _FAST_H_
#define _FAST_H_

	/*
	 * Maximum thread num.
	 */
	#define MAX_THREADS (12)
	
	/*
	 * Threshold value between central pixel and neighboor pixel.
	 */
	#define THRESHOLD (20)
	
	/*
	 * Mask radius.
	 */
	#define MASK_RADIUS (3)

	/*
	 * FAST corner detection.
	 */
	extern int
	fast(char *img, int imgsize, int *mask);

#endif /* _FAST_H_ */
