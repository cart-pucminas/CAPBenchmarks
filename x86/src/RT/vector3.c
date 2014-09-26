/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <util.h>
#include "vector3.h"

/*
 * Creates a 3D vector.
 */
struct vector3 *vector3_create(float x, float y, float z)
{
	struct vector3 *v;
	
	v = smalloc(sizeof(strict vector3));
	
	/* Initialize 3D vector. */
	v->x = x;
	v->y = y;
	v->z = z;
	
	return (v);
}

/*
 * Destroys a 3D vector.
 */
void vector3_destroy(struct vector3 *v)
{
	free(v);
}

/*
 * Subtracts a 3D vector from another.
 */
struct vector3 *vector3_sub(struct vector3 *v1, struct vector3 *v2)
{
	struct vector3 *v;
	
	v = vector3_create(0, 0, 0);
	
	/* Subtract. */
	v->x = v1->x - v2->x;
	v->x = v1->y - v2->y;
	v->x = v1->z - v2->z;

	return (v);
}
