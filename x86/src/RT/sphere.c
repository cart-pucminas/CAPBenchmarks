/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <assert.h>
#include <math.h>
#include <sphere.h>
#include <util.h>
#include <vector.h>

/*
 * Returns the center of a sphere.
 */
vector_t sphere_center(struct sphere *s)
{
	/* Sanity check. */
	assert(s != NULL);
	
	return (s->center);
}

/*
 * Creates a sphere.
 */
struct sphere *sphere_create
(vector_t c, float r, vector_t sc, float rf, float t, vector_t ec)
{
	struct sphere *s;
	
	/* Sanity check. */
	assert(c != NULL);
	assert(r > 0);
	assert(t >= 0);
	assert(t >= 0);
	assert(sc != NULL);
	assert(ec != NULL);
	
	s = smalloc(sizeof(struct sphere));
	
	/* Initialize sphere. */
	s->radius = r;
	s->radius2 = r*r;
	s->transparency = t;
	s->reflection = rf;
	s->surface_color = vector_clone(sc);
	s->emission_color = vector_clone(ec);
	s->center = vector_clone(c);
	
	return (s);
}

/*
 * Destroys a sphere.
 */
void sphere_destroy(struct sphere *s)
{
	/* Sanity check. */
	assert(s != NULL);
	
	/* Deallocate internal data. */
	vector_destroy(s->surface_color);
	vector_destroy(s->emission_color);
	vector_destroy(s->center);
	
	free(s);
}

/*
 * Asserts if a ray intercepts a sphere.
 */
int sphere_intersects
(struct sphere *s, vector_t rayorig, vector_t raydir, float *t0, float *t1)
{
	float d2;
	float tca;
	float thc;
	vector_t l;
	
	/* Sanity check. */
	assert(s != NULL);
	assert(rayorig != NULL);
	assert(raydir != NULL);
	
	l = vector_clone(s->center);
	vector_sub(l, rayorig);
	
	tca = vector_dot(l, raydir);
	
	if (tca < 0) {
		vector_destroy(l);
		return (0);
	}
	
	d2 = vector_dot(l, l) - tca*tca;
	
	if (d2 > s->radius2) {
		vector_destroy(l);
		return (0);
	}
	
	thc = sqrt(s->radius2 -d2);
	
	if ((t0 != NULL) && (t1 != NULL)) {
			*t0 = tca - thc;
			*t1 = tca + thc;
	}
	
	vector_destroy(l);
	return (1);
}
