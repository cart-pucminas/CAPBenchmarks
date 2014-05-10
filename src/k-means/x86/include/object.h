/*
 * Copyright (C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * <object.h> - Object library.
 */

#ifndef OBJECT_H_
#define OBJECT_H_

	#include <stdio.h>

	/*
	 * Object information.
	 */
	struct objinfo
	{
		/* Object functions. */
		void *(*read)(FILE *);         /* Read.         */
		void (*write)(FILE *, void *); /* Write.        */
		int (*cmp)(void *, void *);    /* Compare keys. */
		void *(*getkey)(void *);       /* Get key.      */
		int (*hash)(void *);           /* Compute hash. */
		void *(*clone)(void *);        /* Clone object. */
	};
	
	/*
	 * Clones an object.
	 */
	#define CLONE(info, obj) ((info)->clone(obj))
	
	/*
	 * Compares two keys.
	 */
	#define CMP(info, key1, key2) ((info)->cmp(key1, key2))
	
	/*
	 * Gets object's key.
	 */
	#define GETKEY(info, obj) ((info)->getkey(obj))
	
	/*
	 * Computes hash value.
	 */
	#define HASH(info, key) ((info)->hash(key))
	
	/*
	 * Writes object to a file.
	 */
	#define WRITE(info, file, obj) ((info)->write((file), (obj)))
	
	/*
	 * Integer object information.
	 */
	extern const struct objinfo integer;
	
	/*
	 * Pointer object information.
	 */
	extern const struct objinfo pointer;
	
	/*
	 * String object information.
	 */
	extern const struct objinfo string;
	
	/*
	 * Array object information.
	 */
	extern const struct objinfo array;
	
	/*
	 * Vector object information.
	 */
	extern const struct objinfo vector;

#endif /* OBJECT_H_ */
