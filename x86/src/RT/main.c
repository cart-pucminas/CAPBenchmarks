/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sphere.h>
#include <vector.h>

extern void render(sphere_t *spheres, int nspheres);

/*
 * RT kernel.
 */
int main(int argc, char **argv)
{
	sphere_t spheres[3];
	
	
	vector_t s1_scolor;
	vector_t s1_center;
	vector_t s1_ecolor;
	
	vector_t s2_scolor;
	vector_t s2_center;
	vector_t s2_ecolor;
	
	
	vector_t s3_scolor;
	vector_t s3_center;
	vector_t s3_ecolor;
	
	((void)argc);
	((void)argv);
	
	s1_center = vector_create(0, -10004, -20);
	s1_scolor = vector_create(0.2, 0.2, 0.2);
	s1_ecolor = vector_create(0, 0, 0);
	spheres[0] = sphere_create(s1_center, 10000, s1_scolor, 0, 0, s1_ecolor);
	
	s2_center = vector_create(0, 0, -20);
	s2_scolor = vector_create(1.00, 0.32, 0.36);
	s2_ecolor = vector_create(0, 0, 0);
	spheres[1] = sphere_create(s2_center, 4, s2_scolor, 1, 0.5, s2_ecolor);
	
	s3_center = vector_create(0, 30, -30);
	s3_scolor = vector_create(0, 0, 0);
	s3_ecolor = vector_create(3, 3, 3);
	spheres[2] = sphere_create(s3_center, 3, s3_scolor, 0, 0, s3_ecolor);
	
	render(spheres, 3);
	
	vector_destroy(s1_scolor);
	vector_destroy(s1_center);
	vector_destroy(s1_ecolor);
	sphere_destroy(spheres[0]);
	vector_destroy(s2_scolor);
	vector_destroy(s2_center);
	vector_destroy(s2_ecolor);
	sphere_destroy(spheres[1]);
	vector_destroy(s3_scolor);
	vector_destroy(s3_center);
	vector_destroy(s3_ecolor);
	sphere_destroy(spheres[2]);
	
	return (EXIT_SUCCESS);
}
