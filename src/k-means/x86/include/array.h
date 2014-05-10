/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <array.h> - Array library.
 */

#ifndef ARRAY_H_
#define ARRAY_H_

	#include <object.h>

	/*
	 * Array of objects.
	 */
	struct array
	{
		int size;                   /* Size.               */
		void **objects;             /* Stored objects.     */
		const struct objinfo *info; /* Object information. */
	};
	
	/*
	 * Opaque pointer to an array of objects.
	 */
	typedef struct array * array_t;
	
	/*
	 * Creates an array of objects.
	 */
	extern array_t array_create(const struct objinfo *info, int size);
	
	/*
	 * Destroys an array of objects.
	 */
	extern void array_destroy(array_t a);
	
	/*
	 * Returns the size of an array.
	 */
	#define array_size(a) \
		(((array_t)(a))->size)
	
	/*
	 * Searches for an object in an array of objects.
	 */
	extern int array_search(array_t a, void *key);
	
	/*
	 * Returns the i-th object of an array of objects.
	 */
	#define ARRAY(a, i) \
		(((array_t)(a))->objects[(i)])

#endif /* ARRAY_H_ */
