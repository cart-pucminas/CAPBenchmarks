/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef VECTOR3_H_
#define VECTOR3_H_

	/*
	 * 3D vector.
	 */
	struct vector3
	{
		float x; /* X axis. */
		float y; /* Y axis. */
		float z; /* Z axis. */
	};
	
	/*
	 * Opaque pointer to a 3D vector.
	 */
	typedef struct vector3 * vector3_t;
	
	/*
	 * Creates a 3D vector.
	 */
	extern vector3_t vector3_create(float x, float y, float z);

	/*
	 * Adds a 3D vector to another.
	 */
	#define vector3_add(v1, v2) \
		{ v1->x += v2->x; v1->y += v2->y, v1->z += v2->z}

	/*
	 * Cross product.
	 */
	#define vector3_cross(v1, v2) \
		(vector3_create(v1->x*v2->x, v1->y*v2->y, v1->z*v2->z))
	
	/*
	 * Destroys a 3D vector.
	 */
	extern void vector3_destroy(vector3_t v);	
	
	/*
	 * Dot product.
	 */
	#define vector3_dot(v1, v2) \
		(v1->x*v2->x + v1->y*v2->y + v1->z*v2->z)
	
	/*
	 * Computers the Euclidean distance between two 3d vectors.
	 */
	#define vector3_distance(v1, v2) \
		(sqrt(vector3_dot(v1, v2)))

	/*
	 * Multiples a 3D vector by -1.
	 */
	#define vector3_minus(v) \
		{ v->x = - v->x; v->y = - v->y; v->z = - v->z}

	/*
	 * Multiples a 3D vector by a scalar.
	 */
	#define vector3_mult(v, s) \
		{ v->x *= s; v->y *= s; v->z *= s}

	/*
	 * Subtracts a 3D vector from another.
	 */
	#define vector3_sub(v1, v2) \
		{ v1->x -= v2->x; v1->y -= v2->y, v1->z -= v2->z}

#endif /* VECTOR3_H_ */
