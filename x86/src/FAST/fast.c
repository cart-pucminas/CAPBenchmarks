/*
 * Copyright(C) 2014 Alyson D. Pereira <alyson.deives@outlook.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * fast.c - FAST corner detection kernel.
 */

#include <global.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "fast.h"

/*
 * FAST corner detection.
 */
 
int fast(char *img, int imgsize, int *mask)
{
	int i,j,k,r,z,x,y;
	char accumBrighter, accumDarker;
	char imagePixel,centralPixel;
	int corners[MAX_THREADS] = {0};
	int numcorners = 0;
	
	#pragma omp parallel default(shared) private(imagePixel,centralPixel,i,j,r,x,y,accumBrighter,accumDarker)
	{
		#pragma omp for
		for (i = 0; i< imgsize; i++){
			for (j = 0; j< imgsize; j++){
				centralPixel = img[j*imgsize + i];
				z = 0;
				while(z<16){
					accumBrighter = 0;
					accumDarker = 0;
					for(r = 0;r<9;r++){
						x = i + mask[(r+z) * 2 + 0];
						y = j + mask[(r+z) * 2 + 1];

						if(x >=0 && x < imgsize && y >=0 && y < imgsize){
							imagePixel = img[y * imgsize + x];
							if(imagePixel >= (centralPixel+THRESHOLD) && accumBrighter == 0){
								accumDarker++;
							}
							else if (imagePixel<=(centralPixel-THRESHOLD) && accumDarker == 0){
								accumBrighter++;
							}
							else{
								goto not_a_corner;
							}
						}
					}
					if(accumBrighter == 9 || accumDarker == 9){
						corners[omp_get_thread_num()]++;
						z = 16;
					}
not_a_corner:		z++;
				}
			}
		}
	}
	
	for(k=0;k<MAX_THREADS;k++){
		numcorners += corners[k];
	}

	return numcorners;
}
