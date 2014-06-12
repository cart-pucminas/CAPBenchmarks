/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/master/work.c - Work library.
 */

#include <assert.h>
#include <global.h>
#include <message.h>
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
		/* SORTWORK. */
		case SORTWORK :
			msg->u.sortwork.id = va_arg(ap, int);
			msg->u.sortwork.size = va_arg(ap, int);
			msg->type = SORTWORK;
			break;
		
		/* SORTRESULT. */
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
	communication += data_send(outfd, msg, sizeof(struct message));
}

/*
 * Receives a message.
 */
struct message *message_receive(int infd)
{
	struct message *msg;
	
	msg = message_create(DIE);
	
	communication += data_receive(infd, msg, sizeof(struct message));
	
	return (msg);
}
