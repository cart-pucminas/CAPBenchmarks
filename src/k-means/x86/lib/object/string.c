/*
 * Copyright(C) 2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * object/string.c - String object information.
 */

#include <object.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * Compares two strings.
 */
static int string_cmp(void *key1, void *key2)
{
	return (strcmp(key1, key2));
}

/*
 * Gets string key.
 */
static void *string_getkey(void *obj)
{
	return (obj);
}

/*
 * Writes string to a file.
 */
static void string_write(FILE *file, void *obj)
{
	fputs(obj, file);
}

/*
 * Computes the hash value of a string using the dbj2 algorithm, proposed by 
 * Dan Bernstein.
 */
static unsigned dbj2(char *str)
{
	int c;
	unsigned hash;

	hash = 5381;

	/* Compute hash value. */
	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return (hash);
}

#ifdef XXX

/*
 * 
 */
static unsigned sdbm(char *str)
{
	int c;
	unsigned hash;
	
	hash = 0;

	/* Compute hash value. */
	while (c = *str++)
		hash = c + (hash << 6) + (hash << 16) - hash;

	return (hash);
}

/*
 * 
 */
static unsigned kandR(char *str)
{
	int c;
	unsigned hash;
	
	hash = 0;

	/* Compute hash value. */
	while (c = *str++)
	    hash += c;

	return (hash);
}

#endif

/*
 * Computes hash value.
 */
static int string_hash(void *key)
{
	int hash;
	
	hash = (int) dbj2(key);
	
	return ((hash < 0) ? hash * -1 : hash);
}

/*
 * Allocates a string.
 */
void *string_clone(void *obj)
{
	char *str;
	
	str = malloc((strlen(obj) + 1)*sizeof(char));
	
	/* Failed to allocate new string. */
	if (str == NULL)
		return (NULL);
	
	return (strcpy(str, obj));
}


/*
 * String object information.
 */
const struct objinfo string =
{
	NULL,          /* read()   */
	string_write,  /* write()  */
	string_cmp,    /* cmp()    */
	string_getkey, /* getkey() */
	string_hash,   /* hash()   */
	string_clone   /* clone()  */
};

