/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <float.h>
#include "sphere.h"
#include "vector3.h"

#define INFINITY FLT_MAX

/*
 * Traces a ray into a scene.
 */
vector3_t trace
(vector3_t rayorig, vector3_t raydir, sphere_t *spheres, int nspheres)
{
	int i;       /* Loop index.                  */
	float t0;    /* First intersection point.    */
	float t1;    /* Second intersection point.   */
	sphere_t s;  /* Closest object in the scene. */
	float tnear; /* Closest intersection point.  */
	
	/* No object intercepts the scene. */
	s = NULL;
	tnear = INFINITY;
	
	/* Find closest object in the scene. */
	for (i = 0; i < nspheres; i++)
	{
		t0 = INFINITY; t1 = INFINITY;
		
		/* Intersects. */
		if (sphere_intersect(spheres[i], rayorign, raydir, &t0, &t1))
		{
			/* Use the second intersection point. */
			if (t0 < 0)
				t0 = t1;
			
			/* Closest object found. */
			if (t0 < tnear)
			{
				tnear = t0;
				s = spheres[i];
			}
		}
	}
	
	/*
	 * There is no intersection point.
	 * Return black.
	 */
	if (s == NULL)
		return (vector_create(0, 0, 0));
}
