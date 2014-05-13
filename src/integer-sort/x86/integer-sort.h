/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * Integer Sort library.
 */

#ifndef INTEGER_SORT_H_
#define INTEGER_SORT_H_

	#include <stdlib.h>
	#include <stdio.h>

/*============================================================================*
 *                              Utility Library                               *
 *============================================================================*/

	/*
	 * Prints an error message and exits.
	 */
	extern void error(const char *msg);

	/*
	 * Allocates memory safely.
	 */
	extern void *smalloc(size_t size);

/*============================================================================*
 *                            Linked List Library                             *
 *============================================================================*/
 
	/*
	 * List node.
	 */
	struct node
	{
		int x;             /* Number.                */
		struct node *next; /* Next node in the list. */
	};

	/*
	 * Linked list.
	 */
	struct list
	{
		int length;       /* List length. */
		struct node head; /* Head node.   */
		struct node tail; /* Tail node.   */
	};

	/*
	 * Casters to a list pointer.
	 */
	#define LISTP(l) \
		((struct list *)(l))

	/*
	 * Creates a list of numbers.
	 */
	extern struct list *list_create(void);
	
	/*
	 * Destroys a list of numbers.
	 */
	extern void list_destroy(struct list *l);

	/*
	 * Asserts if a list is empty.
	 */
	#define list_empty(l) \
		(LISTP(l)->head.next == &LISTP(l)->tail)
	
	/*
	 * Pushes a number in a list.
	 */
	extern void list_push(struct list *l, int x);
	
	/*
	 * Pops a number from a list.
	 */
	extern int list_pop(struct list *l);


/*============================================================================*
 *                                Global Library                              *
 *============================================================================*/

	/*
	 * Number of threads.
	 */
	extern int nthreads;

#endif /* INTEGER_SORT_H_ */
