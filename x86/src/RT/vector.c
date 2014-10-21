/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <assert.h>
#include <math.h>
#include <util.h>
#include <vector.h>

/*
 * Adds v2 to v1.
 */
void vector_add(struct vector *v1, struct vector *v2)
{
	/* Sanity check. */
	assert(v1 != NULL);
	assert(v2 != NULL);
	
	v1->x += v2->x;
	v1->y += v2->y;
	v1->z += v2->z;
}

/*
 * Clones a vector.
 */
struct vector *vector_clone(struct vector *v)
{
	/* Sanity check. */
	assert(v != NULL);
	
	return (vector_create(v->x, v->y, v->z));
}
	
/*
 * Creates a 3D vector.
 */
struct vector *vector_create(float x, float y, float z)
{
	struct vector *v;
	
	v = smalloc(sizeof(struct vector));
	
	/* Initialize vector. */
	v->x = x;
	v->y = y;
	v->z = z;
	
	return (v);
}

/*
 * Cross product.
 */
struct vector *vector_cross(struct vector *v1, struct vector *v2)
{
	struct vector *v;
	
	/* Sanity check. */
	assert(v1 != NULL);
	assert(v2 != NULL);
	
	v = vector_create(v1->x*v2->x, v1->y*v2->y, v1->z*v2->z);
	
	return (v);
}

/*
 * Destroys a 3D vector.
 */
void vector_destroy(struct vector *v)
{
	free(v);
}

/*
 * Dot product.
 */
float vector_dot(struct vector *v1, struct vector *v2)
{
	/* Sanity check. */
	assert(v1 != NULL);
	assert(v2 != NULL);
	
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
}

/*
 * Multiplies a 3D vector by -1.
 */
void vector_invert(struct vector *v)
{
	/* Sanity check. */
	assert(v != NULL);
	
	v->x = -v->x;
	v->y = -v->y;
	v->z = -v->z;
}

/*
 * Normalizes a 3D vector.
 */
void vector_normalize(struct vector *v)
{
	float norm2;    /* normal^2  */
	float norminv;  /* normal^-1 */
	
	/* Sanity check. */
	assert(v != NULL);
	
	norm2 = v->x*v->x + v->y*v->y + v->z*v->z;
	
	if (norm2 == 0) {
		return;
	}
	
	norminv = 1 / sqrt(norm2);
	v->x *= norminv;
	v->y *= norminv;
	v->z *= norminv;
}

/*
 * Scalar product.
 */
void vector_scalar(struct vector *v, float a)
{
	/* Sanity check. */
	assert(v != NULL);
	
	v->x *= a;
	v->y *= a;
	v->z *= a;
}

/*
 * Subtracts v2 from v1.
 */
void vector_sub(struct vector *v1, struct vector *v2)
{
	/* Sanity check. */
	assert(v1 != NULL);
	assert(v2 != NULL);
	
	v1->x -= v2->x;
	v1->y -= v2->y;
	v1->z -= v2->z;
}
