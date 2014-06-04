/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

	#include <stdlib.h>

	/* Message types. */
	#define SORTWORK   0 /* Sort array.        */
	#define SORTRESULT 1 /* Sort array result. */
	#define DIE        3 /* Die.                */
	
	/*
	 * Message.
	 */
	struct message
	{
		int type; /* Message type (see above). */
		
		union
		{
			/* SORTWORK. */
			struct 
			{
				int id;   /* Bucket ID.       */
				int size; /* Minibucket size. */
			} sortwork;
			
			/* SORTRESULT. */
			struct
			{
				int id;   /* Bucket ID.       */
				int size; /* Minibucket size. */
			} sortresult;
		} u;
	};
	
	/*
	 * Receives data.
	 */
	extern void data_receive(int infd, void *data, size_t n);
	
	/*
	 * Sends data.
	 */
	extern void data_send(int outfd, void *data, size_t n);
	
	/*
	 * Creates a message.
	 */
	extern struct message *message_create(int type, ...);
	
	/*
	 * Destroys a message.
	 */
	extern void message_destroy(struct message *msg);
	
	/*
	 * Receives a message.
	 */
	extern struct message *message_receive(int infd);
	
	/*
	 * Sends a message.
	 */
	extern void message_send(int outfd, struct message *msg);

#endif /* MESSAGE_H_ */
