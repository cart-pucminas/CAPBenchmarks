/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/master/work.c - Work library.
 */

#include <assert.h>
#include <message.h>
#include <mppaipc.h>
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
		case SORTWORK :
			msg->u.sortwork.id = va_arg(ap, int);
			msg->u.sortwork.size = va_arg(ap, int);
			msg->type = SORTWORK;
			break;
		
		/* FINDRESULT. */
		case SORTRESULT :
			msg->u.sortresult.id = va_arg(ap, int);
			msg->u.sortresult.size = va_arg(ap, int);
			msg->type = SORTRESULT;
			break;
		
		
		/* DIE. */
		default :
			msg->type = DIE;
			break;
	}
	
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
