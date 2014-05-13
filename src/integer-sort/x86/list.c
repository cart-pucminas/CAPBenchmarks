/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include "integer-sort.h"

/*
 * Creates a node.
 */
static struct node *node_create(int x)
{
	struct node *newnode;
	
	newnode = smalloc(sizeof(struct node));
	
	/* Initialize node. */
	newnode->x = x;
	newnode->next = NULL;
	
	return (newnode);
}

/*
 * Destroys a node.
 */
static void node_destroy(struct node *node)
{
	free(node);
}

/*
 * Creates a list of numbers.
 */
struct list *list_create(void)
{
	struct list *l;
	
	l = smalloc(sizeof(struct list));
	
	/* Initialize list. */
	l->length = 0;
	l->head.next = &l->tail;
	l->tail.next = &l->tail;
	
	return (l);
}

/*
 * Destroys a list of numbers.
 */
void list_destroy(struct list *l)
{
	/* Drop all numbers from the list. */
	while (!list_empty(l))
		list_pop(l);
	
	/* House keeping. */
	free(l);
}

/*
 * Pushes a number in a list.
 */
void list_push(struct list *l, int x)
{
	struct node *node;
	
	node = node_create(x);
	
	/* Insert node. */
	node->next = l->head.next;
	l->head.next = node;
	l->length++;
}

/*
 * Pops a number from a list.
 */
int list_pop(struct list *l)
{
	int x;             /* Number. */
	struct node *node; /* Node.   */
	
	if (list_empty(l))
		return (-1);
	
	node = l->head.next;
	l->head.next = node->next;
	x = node->x;
	l->length--;
	
	node_destroy(node);
	
	return (x);
}
