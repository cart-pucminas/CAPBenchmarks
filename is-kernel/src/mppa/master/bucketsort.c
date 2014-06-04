/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <global.h>
#include <limits.h>
#include <message.h>
#include <util.h>
#include "master.h"

/* Number of buckets. */
#define NUM_BUCKETS 256

/*
 * Bucket-sort algorithm.
 */
extern void bucketsort(int *array, int n)
{
	int max;                  /* Maximum number.      */
	int i, j;                 /* Loop indexes.        */
	int range;                /* Bucket range.        */
	struct minibucket *minib; /* Working mini-bucket. */
	struct message *msg;      /* Working message.     */
	struct bucket **todo;     /* Todo buckets.        */
	struct bucket **done;     /* Done buckets.        */
	
	/* Setup slaves. */
	open_noc_connectors();
	spawn_slaves();
	sync_slaves();
	
	todo = smalloc(NUM_BUCKETS*sizeof(struct bucket *));
	done = smalloc(NUM_BUCKETS*sizeof(struct bucket *));
	for (i = 0; i < NUM_BUCKETS; i++)
	{
		done[i] = bucket_create();
		todo[i] = bucket_create();
	}
	
	
	/* Find max number in the array. */
	max = INT_MIN;
	for (i = 0; i < n; i++)
	{
		/* Found. */
		if (array[i] > max)
			max = array[i];
	}
	
	/* Distribute numbers. */
	range = max/NUM_BUCKETS;
	for (i = 0; i < n; i++)
	{
		j = array[i]/range;
		if (j >= NUM_BUCKETS)
			j = NUM_BUCKETS - 1;
		
		bucket_insert(&todo[j], array[i]);
	}
	
	/* Sort buckets. */
	j = 0;
	for (i = 0; i < NUM_BUCKETS; i++)
	{
		while (bucket_size(todo[i]) > 0)
		{
			minib = bucket_pop(todo[i]);
			
			/* Send message. */
			msg = message_create(SORTWORK, i, minib->size);
			message_send(outfd[j], msg);
			message_destroy(msg);
			
			/* Send data. */
			data_send(outfd[j], minib->elements, minib->size*sizeof(int));
			minibucket_destroy(minib);
			
			j++;
			
			/* 
			 * Slave processes are busy.
			 * So let's wait for results.
			 */
			if (j == nthreads)
			{
				/* Receive results. */
				for (/* NOOP */ ; j > 0; j--)
				{					
					/* Receive message. */
					msg = message_receive(infd[nthreads - j]);
					
					/* Receive mini-bucket. */
					minib = minibucket_create();
					minib->size = msg->u.sortresult.size;
					data_receive(infd[nthreads -j], minib->elements, 
													minib->size*sizeof(int));
					
					message_destroy(msg);
					
					bucket_push(done[msg->u.sortresult.id], minib);
				}
			}
		}
	}
	
	/* Receive results. */
	for (/* NOOP */ ; j > 0; j--)
	{						
		/* Receive message. */
		msg = message_receive(infd[j - 1]);
					
		/* Receive bucket. */
		minib = minibucket_create();
		minib->size = msg->u.sortresult.size;
		data_receive(infd[j - 1], minib->elements, minib->size*sizeof(int));
					
		message_destroy(msg);
					
		bucket_push(done[msg->u.sortresult.id], minib);
	}
	
	/* Rebuild array. */
	j = 0;
	for (i = 0; i < NUM_BUCKETS; i++)
	{
		bucket_merge(done[i], &array[j]);
		j += bucket_size(done[i]);
	}
	
	/* House keeping. */
	for (i = 0; i < NUM_BUCKETS; i++)
	{
		bucket_destroy(todo[i]);
		bucket_destroy(done[i]);
	}
	free(done);
	free(todo);
	join_slaves();
	close_noc_connectors();
}
