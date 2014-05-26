/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/master/work.c - Work library.
 */

#include <arch.h>
#include <assert.h>
#include <stdarg.h>
#include <util.h>

/*
 * Creates a message.
 */
struct message *message_create(int type, ...)
{
	va_list ap;          /* Arguments pointer. */
	struct message *msg; /* Message.           */
	
	va_start(ap, type);
	
	msg = smalloc(sizeof(struct message));
	
	/* Parse type of message. */
	switch (type)
	{
		/* FINDWORK. */
		case FINDWORK :
			msg->u.findwork.i0 = va_arg(ap, int);
			msg->u.findwork.j0 = va_arg(ap, int);
			msg->u.findwork.height = va_arg(ap, int);
			msg->u.findwork.width = va_arg(ap, int);
			msg->type = FINDWORK;
			break;
		
		/* FINDRESULT. */
		case FINDRESULT :
			msg->u.findresult.i0 = va_arg(ap, int);
			msg->u.findresult.j0 = va_arg(ap, int);
			msg->u.findresult.ipvt = va_arg(ap, int);
			msg->u.findresult.jpvt = va_arg(ap, int);
			msg->type = FINDRESULT;
			break;
		
		/* REDUCTWORK. */
		case REDUCTWORK :
			msg->u.reductwork.ipvt = va_arg(ap, int);
			msg->u.reductwork.i0 = va_arg(ap, int);
			msg->u.reductwork.j0 = va_arg(ap, int);
			msg->u.reductwork.height = va_arg(ap, int);
			msg->u.reductwork.width = va_arg(ap, int);
			msg->type = REDUCTWORK;
			break;
		
		/* REDUCTRESULT. */
		case REDUCTRESULT :
			msg->u.reductresult.i0 = va_arg(ap, int);
			msg->u.reductresult.j0 = va_arg(ap, int);
			msg->u.reductresult.height = va_arg(ap, int);
			msg->u.reductresult.width = va_arg(ap, int);
			msg->type = REDUCTRESULT;
			break;
			
		
		/* DIE. */
		default :
			msg->type = DIE;
			break;
	}
	
	msg->next = NULL;
	
	va_end(ap);
	
	return (msg);
}

/*
 * Destroys a message.
 */
void message_destroy(struct message *msg)
{
	free(msg);
}

/*
 * Sends a message.
 */
void message_send(int outfd, struct message *msg)
{
	ssize_t count;
	
	count = mppa_write(outfd, msg, sizeof(struct message));
	assert(count != -1);
}

/*
 * Receives a message.
 */
struct message *message_receive(int infd)
{
	ssize_t count;       /* Bytes read. */
	struct message *msg; /* Message.    */
	
	msg = message_create(DIE);
	
	count = mppa_read(infd, msg, sizeof(struct message));
	assert(count != -1);
	
	return (msg);
}

/*
 * Sends data.
 */
void data_send(int outfd, void *data, size_t n)
{	
	ssize_t count;
	
	count = mppa_write(outfd, data, n);
	assert(count != -1);
}

/*
 * Receives data.
 */
void data_receive(int infd, void *data, size_t n)
{	
	ssize_t count;
	
	count = mppa_read(infd, data, n);
	assert(count != -1);
}
