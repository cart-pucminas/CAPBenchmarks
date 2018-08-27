#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>

/*
 * Prints an error message and exits.
 */
extern void error(const char *msg);

/*
 * Safe calloc().
 */
extern void *scalloc(size_t nmemb, size_t size);

/*
 * Safe malloc().
 */
extern void *smalloc(size_t size);

#endif