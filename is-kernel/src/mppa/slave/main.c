/*
 * Copyright (C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/slave/main.c - Slave main().
 */

#include <arch.h>
#include <assert.h>
#include <global.h>
#include <message.h>
#include <omp.h>
#include <stdlib.h>
#include "slave.h"

/* 
 * Array block.
 */
struct 
{
	int size;                                   /* Size of block. */
	int elements[CLUSTER_WORKLOAD/sizeof(int)]; /* Elements.      */
} block;

/*
 * Sorts an array of numbers.
 */
extern void sort(int *a, int n);

/*
 * Obeys master.
 */
int main(int argc, char **argv)
{
	int id;              /* Bucket ID.        */
	struct message *msg; /* Message.          */
	
	((void)argc);
	
	rank = atoi(argv[0]);
	open_noc_connectors();
	sync_master();
	
	/* Slave life. */
	while (1)
	{
		msg = message_receive(infd);
		
		switch (msg->type)
		{
			/* SORTWORK. */
			case SORTWORK:
				/* Receive matrix block. */
				block.size = msg->u.sortwork.size;
				data_receive(infd, block.elements, block.size*sizeof(int));
				
				/* Extract message information. */
				id = msg->u.sortwork.id;
				message_destroy(msg);
				
				sort(block.elements, block.size);
				
				/* Send message back.*/
				msg = message_create(SORTRESULT, id, block.size);
				message_send(outfd, msg);
				data_send(outfd, block.elements, block.size*sizeof(int));
				message_destroy(msg);
				
				break;
				
			/* DIE. */
			default:
				message_destroy(msg);
				goto out;		
		}
	}

out:

	close_noc_connectors();
	mppa_exit(0);
	return (0);
}
