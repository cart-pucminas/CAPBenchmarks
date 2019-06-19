/* Kernel Includes */
#include <util.h>
#include "master.h"

/* C And MPPA Library Includes*/
#include <stdlib.h>

/*
 * Creates a mini-bucket.
 */
struct minibucket *minibucket_create(void)
{
    struct minibucket *minib;

    minib = smalloc(sizeof(struct minibucket));

    /* Initialize mini-bucket. */
    minib->size = 0;
    minib->next = NULL;

    return (minib);
}

/*
 * Destroys a mini bucket.
 */
void minibucket_destroy(struct minibucket *minib)
{
    free(minib);
}
