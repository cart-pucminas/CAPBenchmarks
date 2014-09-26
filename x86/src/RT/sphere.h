/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef SPHERE_H_
#define SPHERE_H_

	#include "vector3.h"

	/*
	 * Sphere.
	 */
	struct sphere
	{
		vector3_t center;         /* Center.               */
		float radius;             /* Radius.               */
		float radius2;            /* Radius squared.       */
		vector3_t surface_color;  /* Surface color.        */
		vector3_t emission_color; /* Emission color.       */
		float transparency;       /* Surface transparency. */
		float reflection;         /* Surface reflection.   */
		
	};
	
	/*
	 * Opaque pointer to a sphere.
	 */
	typedef struct sphere *sphere_t;
	
	/*
	 * Creates a sphere.
	 */
	extern sphere_t *sphere_create(vector3_t c, float r, vector3_t sc);
	
	/*
	 * Destroys a sphere.
	 */
	extern void sphere_destroy(vector3_t s);
	
	/*
	 * Computers a ray-intersection using geometric solution.
	 */
	extern int sphere_intersect
	(sphere_t s, vector3_t rayorig, vector3_t raydir, float *t0, float *t1);

#endif /* SPHERE_H_ */
