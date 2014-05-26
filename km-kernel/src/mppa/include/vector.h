/*
 * Copyright(C) 2011-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <vector.h> - Vector library.
 */

#ifndef VECTOR_H_
#define VECTOR_H_

	/*
	 * Calculates the distance between two points.
	 */
	extern float vector_distance(float *a, float *b);
	
	/*
	 * Adds two vectors.
	 */
	extern float *vector_add(float *v1, const float *v2);
	
	/*
	 * Multiplies a vector by a scalar.
	 */
	extern float *vector_mult(float *v, float scalar);
	
	/*
	 * Assigns a vector to another.
	 */
	extern float *vector_assign(float *v1, const float *v2);
	
	/*
	 * Tests if two vectors are equal.
	 */
	extern int vector_equal(const float *v1, const float *v2);

#endif /* VECTOR_H_ */
