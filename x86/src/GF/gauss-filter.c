/*
 * Copyright(C) 2014 Matheus M. Queiroz <matheus.miranda.queiroz@gmail.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * gaussian-filter.c - Gaussian filter kernel.
 */

#include <global.h>
#include <assert.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <util.h>

/*
 * Gaussian filter.
 */
void gauss_filter(unsigned char *img, int imgsize, double *mask, int masksize)
{
	int half;
	double pixel;
	unsigned char *newimg;
	int imgI, imgJ, maskI, maskJ;
	
	newimg = scalloc(imgsize * imgsize, sizeof(unsigned char));
	
	#define MASK(i, j) \
		mask[(i)*masksize + (j)]
	
	#define IMG(i, j) \
		img[(i)*imgsize + (j)]
	
	#define NEWIMG(i, j) \
		newimg[(i)*imgsize + (j)]
	
	half = masksize/2;
	
	#pragma omp parallel default(shared) private(imgI,imgJ,maskI,maskJ,pixel)
	{
		#pragma omp for
		for (imgI = half; imgI < imgsize - half; imgI++)
		{			
			for (imgJ = half; imgJ < imgsize - half; imgJ++)
			{
				pixel = 0.0;
				for (maskI = 0; maskI < masksize; maskI++)
					for (maskJ = 0; maskJ < masksize; maskJ++)
						pixel += IMG(imgI + maskI - half, imgJ + maskJ - half) * MASK(maskI, maskJ);
				   
				NEWIMG(imgI, imgJ) = (pixel > 255) ? 255 : (int)pixel;
			}
		}
	}
	
	printf("OUTPUT X86:\n");
	for (imgI = 0; imgI < imgsize; imgI++)
	{			
		for (imgJ = 0; imgJ < imgsize; imgJ++)
			fprintf(stderr, "%d ",	NEWIMG(imgI, imgJ));
		fprintf(stderr, "\n");
	}
	
	free(newimg);
}
