/*
 * Copyright(C) 2014 Matheus M. Queiroz <matheus.miranda.queiroz@gmail.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * x86/gaussian-filter.c - gauss_filter() implementation.
 */

#include <global.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Gaussian filter.
 */
void gauss_filter(unsigned char *img, int imgsize, double *mask, int masksize)
{
	int i, j;
	int half;
	double new_element;
	int imgI, imgJ, maskI, maskJ;
	
	#define MASK(i, j) \
		mask[(i)*masksize + (j)]
	
	#define IMG(i, j) \
		img[(i)*imgsize + (j)]
	
	i = 0; j = 0;
	half = imgsize >> 1;
	
	#pragma omp parallel
	{
        for (imgI = 0; imgI < imgsize; imgI++)
        {
            for (imgJ = 0; imgJ < imgsize; imgJ++)
            {
				new_element = 0.0;
                for (maskI = 0; maskI < imgsize; maskI++)
                {
                    for (maskJ = 0; maskJ < imgsize; maskJ++)
                    {
                        i = (imgI - half < 0) ? imgsize - 1 - maskI : imgI - half;
                        j = (imgJ - half < 0) ? imgsize - 1 - maskJ : imgJ - half;

                        new_element += IMG(i, j)*MASK(maskI, maskJ);
                    }
                }
               
				IMG(imgI, imgJ) = (new_element > 255) ? 255 : (int)new_element;
            }
        }
	}
}
