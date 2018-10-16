/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

/* Kernel Include */
#include <arch.h>

/* C And MPPA Library Includes*/
#include <stdlib.h>

/* Message types. */
#define DIE          0 /* End.                */
#define SORTWORK     1 /* Sort array.         */
#define SORTRESULT   2 /* Sort array result.  */
#define FINDWORK     3 /* Find pivot element. */
#define FINDRESULT   4 /* Find pivot element. */
#define REDUCTWORK   5 /* Row reduction.      */
#define REDUCTRESULT 6 /* Row reduction.      */

/* Asserts if a list is empty. */
#define empty(l)  \
((l) == NULL)     \

/* Pushes a message on a list. */
#define push(l, msg)              \
{ (msg)->next = (l); (l) = msg; } \

/* Pops a message from a list. */
#define pop(l, msg)                 \
{ (msg) = (l); (l) = (msg)->next; } \

/* Return the real index of element [i][j] in the matrix array. */
#define OFFSET(width, i, j) \
(((i) * width) + j)         \

/* Message. */
struct message {
	int type;          /* Message type (see above). */

	union {
	    /* SORTWORK. */
		struct {
			int id;   /* Bucket ID.       */
			int size; /* Minibucket size. */
		} sortwork;

		/* SORTRESULT. */
		struct {
			int id;   /* Bucket ID.       */
			int size; /* Minibucket size. */
		} sortresult;

		/* FINDWORK. */
		struct {
			int i0; /* Block start.  */
			int height; /* Block height. */
		} findwork;

		/* FINDRESULT. */
		struct {
			int ipvt;    /* ith index of pivot. */
			int jpvt;    /* jth index of pivot. */
			int i0, j0;  /* Block start.        */
		} findresult;

		/* REDUCTWORK. */
		struct {
			int ipvt;   /* Row index of pivot. */
			int i0, j0; /* Block start.        */
			int height; /* Block height.       */
			int width;  /* Block width.        */
		} reductwork;
	} u;

	/* Next message of a list. */
	struct message *next;
};

/* Creates a message. */
extern struct message *message_create(int type, ...);

/* Destroys a message. */
extern void message_destroy(struct message *msg);

/* Sends a message. */
extern void message_put(struct message *msg, mppa_async_segment_t *seg, int offset, mppa_async_event_t *event);

/* Receives a message. */
extern struct message *message_get(mppa_async_segment_t *seg, int offset, mppa_async_event_t *event);

#endif /* MESSAGE_H_ */
