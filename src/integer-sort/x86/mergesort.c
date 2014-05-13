/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <limits.h>
#include "integer-sort.h"

/*
 * Merges two lists.
 */
inline struct node *merge(struct node *a, struct node *b, struct node *tail)
{
	struct node *c;
	
	tail->x = INT_MAX;
	
	c = tail;
	
	do
	{
		if (a->x <= b->x)
		{
			c->next = a;
			c = a;
			a = a->next;
		}
		else
		{
			c->next = b;
			c = b;
			b = b->next;
		}
	} while (c != tail);
	
	c = tail->next;
	tail->next = tail;
	
	return (c);
}

/*
 * Merge sort algorithm.
 */
void mergesort(struct list *l)
{
	int i, N;
	struct node *t;
	struct node *head;
	struct node *tail;
	struct node *todo;
	struct node *a, *b, *c;
	
	tail = &l->tail;
	head = &l->head;
	
	a = tail;
	
	for (N = 1; a != head->next; N = N+N)
	{
		todo = head->next;
		c = head;
		
		while (todo != tail)
		{
			a = t = todo;
			
			for (i = 1; i < N; i++)
				t = t->next;
			b = t->next;
			t->next = tail;
			
			t = b;
			for (i = 1; i < N; i++)
				t = t->next;
			todo = t->next;
			t->next = tail;
			
			c->next = merge(a, b, tail);
			
			for (i = 1; i <= N + N; i++)
				c = c->next;
		}
	}
}
