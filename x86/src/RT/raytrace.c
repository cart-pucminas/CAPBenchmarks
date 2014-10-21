/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <assert.h>
#include <float.h>
#include <math.h>
#include <sphere.h>
#include <stdlib.h>
#include <stdio.h>
#include <util.h>
#include <vector.h>

#define BIAS 0.00001
#define PI 3.141592653589793
#define INFINITY FLT_MAX
#define MAX_RAY_DEPTH 20

static float max(float a, float b)
{
	return ((a > b) ? a : b);
}

static float min(float a, float b)
{
	return ((a < b) ? a : b);
}

static float mix(float a, float b, float mix)
{
	return (b * mix + a * (1.0 - mix));
}

/*
 * 
 */
vector_t raytrace(vector_t rayorig, vector_t raydir, sphere_t *spheres, int nspheres, int depth)
{
	int inside;
	int i, j;               /* Loop indexes. */
	float t0;
	float t1;
	sphere_t s;
	vector_t surface_color; /* Color of the surface.             */
	vector_t phit;          /* Point of intersection.            */
	vector_t nhit;          /* Normal at the intersection point. */
	vector_t lightdir;      /* Light direction. */
	vector_t refldir;
	vector_t refrdir;
	vector_t tmp1, tmp2, tmp3;
	vector_t reflection;
	vector_t refraction;
	float tnear;
	float facingratio;
	float fresneleffect;
	float ior;
	float cosi;
	float k;
	float eta;
	
	t0 = INFINITY;
	t1 = INFINITY;
	tnear = INFINITY;
	s = NULL;
	
	/*
	 * Find closest sphere in the scene 
	 * that the ray intercepts.
	 */
	for (i = 0; i < nspheres; i++)
	{
		/* This sphere is intercepted. */
		if (sphere_intersects(spheres[i], rayorig, raydir, &t0, &t1))
		{
			if (t0 < 0) {
				t0 = t1;
			}
			
			/* Closest sphere found. */
			if (t0 < tnear) {
				tnear = t0;
				s = spheres[i];
			}
		}
	}
	
	/*
	 * There is no intersection
	 * so return background color.
	 */
	if (s == NULL) {
		return (vector_create(2, 2, 2));
	}
	
	
	phit = vector_clone(raydir);
	vector_scalar(phit, tnear);
	vector_add(phit, rayorig);
	
	nhit = vector_clone(phit);
	vector_sub(nhit, sphere_center(s));
	vector_normalize(nhit);
	
	inside = 0;
	if (vector_dot(raydir, nhit) > 0) {
		vector_invert(nhit);
		inside = 1;
	}
	
	tmp3 = vector_clone(nhit);
	vector_scalar(tmp3, BIAS);
	vector_add(tmp3, phit);
	
	if (((s->transparency > 0) || (s->reflection > 0)) && (depth < MAX_RAY_DEPTH))
	{		
		facingratio = -vector_dot(raydir, nhit);
		fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1);
		
		tmp1 = vector_clone(nhit);
		vector_scalar(tmp1, 2*vector_dot(raydir, nhit));
		refldir = vector_clone(raydir);
		vector_sub(refldir, tmp1);
		vector_normalize(refldir);
		vector_destroy(tmp1);
		
		reflection = raytrace(tmp3, refldir, spheres, nspheres, depth + 1);
		vector_destroy(refldir);	
		
		refraction = vector_create(0, 0, 0);
		
		if (s->transparency > 0)
		{
			vector_destroy(refraction);
			
			ior = 1.1;
			eta = (inside) ? ior : 1/ior;
			cosi = -vector_dot(nhit, raydir);
			k = 1 - eta*eta*(1 - cosi*cosi);
			
			refrdir = vector_clone(raydir);
			vector_scalar(refrdir, eta);
			tmp1 = vector_clone(nhit);
			vector_scalar(tmp1, eta*cosi - sqrt(k));
			vector_add(refrdir, tmp1);
			vector_normalize(refrdir);
			vector_destroy(tmp1);
			
			vector_destroy(tmp3);
			tmp3 = vector_clone(nhit);
			vector_scalar(tmp3, BIAS);
			vector_sub(tmp3, phit);
			vector_invert(tmp3);
			
			refraction = raytrace(tmp3, refrdir, spheres, nspheres, depth + 1);
			
			vector_destroy(refrdir);
		}
		
		vector_scalar(refraction, (1 - fresneleffect)*s->transparency);
		vector_scalar(reflection, fresneleffect);
		vector_add(reflection, refraction);
		surface_color = vector_cross(reflection, s->surface_color);
		
		vector_destroy(refraction);
		vector_destroy(reflection);
	}
	
	else
	{	
	surface_color = vector_create(0, 0, 0);
	
	/*
	 * It is a diffuse object, so there
	 * is no need to raytrace any further.
	 */
	for (i = 0; i < nspheres; i++)
	{
		/* This is a light source. */
		if (spheres[i]->emission_color->x > 0)
		{
			int transmission = 1;

			lightdir = vector_clone(spheres[i]->center);
			vector_sub(lightdir, phit);
			vector_normalize(lightdir);
			
			for (j = 0; j < nspheres; j++)
			{	
				if (i == j) {
					continue;
				}
				
				/* Shade this point. */
				if (sphere_intersects(spheres[j], tmp3, lightdir, NULL, NULL))
				{
					transmission = 0;
					break;
				}
			}
			
			if (transmission)
			{
				tmp1 = vector_clone(s->surface_color);
				vector_scalar(tmp1, max(0, vector_dot(nhit, lightdir)));
				tmp2 = vector_cross(tmp1, spheres[i]->emission_color);
				
				vector_add(surface_color, tmp2);
					
				vector_destroy(tmp1);
				vector_destroy(tmp2);
			}
		
			vector_destroy(lightdir);
		}
	}
}
	
	vector_destroy(tmp3);
	vector_destroy(nhit);
	vector_destroy(phit);

	vector_add(surface_color, s->emission_color);

	return (surface_color);
}

void render(sphere_t *spheres, int nspheres)
{
	unsigned width;     /* Image width.    */
	unsigned height;    /* Image height.   */
	vector_t raydir;    /* Ray direction.  */
	vector_t rayorig;   /* Ray origin.     */
	unsigned x, y;
	float xx, yy;
	vector_t *image;
	vector_t *pixel;
	float invwidth;     /* width^-1  */
	float invheight;    /* height^-1 */
	float angle;
	float fov;
	float aspectratio; /* Image's aspect ratio. */

	width = 2560;
	height = 1440;

	image = smalloc(width*height*sizeof(vector_t));
	pixel = image;
	
	invwidth = 1.0 / width;
	invheight = 1.0 /height;
	fov = 30;

	aspectratio = width / ((float)height);
	angle = tan(PI*0.5*fov/180.0);

	rayorig = vector_create(0, 0, 0);

	for (y = 0; y < height; ++y)
	{
		for (x = 0; x < width; x++)
		{
			xx = (2 * ((x + 0.5) * invwidth) - 1) * angle * aspectratio;
			yy = (1 - 2 * ((y + 0.5) * invheight)) * angle;
			
			raydir = vector_create(xx, yy, -1);
			
			vector_normalize(raydir);
	
			
			*pixel++ = raytrace(rayorig, raydir, spheres, nspheres, 0);
			
			vector_destroy(raydir);
		}
	}
}

/*
 * Prints scene.
 */
void scene_print(FILE *file, vector_t *image, unsigned dimension)
{
	unsigned i;
	
	/* Sanity check. */
	assert(file != NULL);
	assert(image != NULL);

	fprintf(output, "P6\n%u %u\n255\n", width, height);
	for (i = 0; i < dimension; i++) {
		fprintf(output, "%c%c%c", 
			(unsigned char)(min(1, image[i]->x)*255),
			(unsigned char)(min(1, image[i]->y)*255), 
			(unsigned char)(min(1, image[i]->z)*255));
			vector_destroy(image[i]);
	}
}
