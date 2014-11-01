/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "sphere.h"
#include "vector.h"

extern image_t render(sphere_t *spheres, int nspheres);

/*
 * RT kernel.
 */
int main(int argc, char **argv)
{
	sphere_t spheres[3];
	image_t img;
	
	struct vector s1_scolor;
	struct vector s1_center;
	struct vector s1_ecolor;
	
	struct vector s2_scolor;
	struct vector s2_center;
	struct vector s2_ecolor;
	
	
	struct vector s3_scolor;
	struct vector s3_center;
	struct vector s3_ecolor;
	
	((void)argc);
	((void)argv);
	
	s1_center = VECTOR(0, -10004, -20);
	s1_scolor = VECTOR(0.2, 0.2, 0.2);
	s1_ecolor = VECTOR(0, 0, 0);
	spheres[0] = sphere_create(s1_center, 10000, s1_scolor, 0, 0, s1_ecolor);
	
	s2_center = VECTOR(0, 0, -20);
	s2_scolor = VECTOR(1.00, 0.32, 0.36);
	s2_ecolor = VECTOR(0, 0, 0);
	spheres[1] = sphere_create(s2_center, 4, s2_scolor, 1, 0.5, s2_ecolor);
	
	s3_center = VECTOR(0, 30, -30);
	s3_scolor = VECTOR(0, 0, 0);
	s3_ecolor = VECTOR(3, 3, 3);
	spheres[2] = sphere_create(s3_center, 3, s3_scolor, 0, 0, s3_ecolor);
	
	img = render(spheres, 3);
	
	image_export("out.ppm", img, IMAGE_PPM);
	
	image_destroy(img);
	
	sphere_destroy(spheres[0]);
	sphere_destroy(spheres[1]);
	sphere_destroy(spheres[2]);
	
	return (EXIT_SUCCESS);
}
