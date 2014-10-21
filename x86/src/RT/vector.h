/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef VECTOR_H_
#define VECTOR_H_

	/*
	 * 3D Vector.
	 */
	struct vector
	{
		float x; /* x coordinate. */
		float y; /* y coordinate. */
		float z; /* z coordinate. */
	};
	
	/*
	 * Opaque pointer to a 3D vector.
	 */
	typedef struct vector * vector_t;
	
	/*
	 * Adds v2 to v1.
	 */
	extern void vector_add(vector_t v1, vector_t v2);
	
	/*
	 * Clones a vector.
	 */
	extern vector_t vector_clone(vector_t v);
	
	/*
	 * Creates a 3D vector.
	 */
	extern vector_t vector_create(float x, float y, float z);
	
	/*
	 * Cross product.
	 */
	extern vector_t vector_cross(vector_t v1, vector_t v2);
	
	/*
	 * Destroys a 3D vector.
	 */
	extern void vector_destroy(vector_t v);
	
	/*
	 * Dot product.
	 */
	extern float vector_dot(vector_t v1, vector_t v2);
	
	/*
	 * Multiplies a 3D vector by -1.
	 */
	extern void vector_invert(vector_t v);
	
	/*
	 * Normalizes a 3D vector.
	 */
	extern void vector_normalize(vector_t v);
	
	/*
	 * Scalar product.
	 */
	extern void vector_scalar(vector_t v, float a);
	
	/*
	 * Subtracts v2 from v1.
	 */
	extern void vector_sub(vector_t v1, vector_t v2);

#endif /* VECTOR_H_ */
