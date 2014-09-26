/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <util.h>
#include "sphere.h"

/*
 * Creates a sphere.
 */
struct sphere *sphere_create(vector3_t c, float r, struct sphere *sc)
{
	struct sphere *s;
	
	s = smalloc(sizeof(struct sphere));
	
	/* Initialize sphere. */
	s->center = c;
	s->radius = r;
	s->radius2 = r*r;
	s->surface_color = sc;
	s->emission_color = vector3_create(0, 0, 0);
	s->transparency = 0;
	s->reflection = 0;
	
	return (s);
}

/*
 * Destroys a sphere.
 */
void sphere_destroy(struct sphere *s)
{
	free(s->emission_color);
	free(s);
}

/*
 * Computers a ray-intersection using geometric solution.
 */
int sphere_intersect
(struct sphere *s, vector3_t rayorig, vector3_t raydir, float *t0, float *t1)
{
	float d2;
	float tca;
	float tch;
	vector3_t l;
	
	l = vector3_sub(s->center, rayorig);
	tca = vector3_dot(l, raydir);
	
	/* Does not intersect. */
	if (tca < 0)
		return (0);
		
	d2 = vector3_dot(l, l) - tca*tca;
	
	/* Does not intersect. */
	if (d2 > s->radius2)
		return (0);
	
	/* Pythagoras's law. */
	tch = sqrt(s->radius2 - d2);
	
	*t0 = tca - tch;
	*t1 = tca + tch;
	
	return (1);
}
