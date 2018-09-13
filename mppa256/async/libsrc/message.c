/* Kernel Include */
#include <async_util.h>
#include <message.h>
#include <global.h>
#include <util.h>

/* C And MPPA Library Includes*/
#include <stdarg.h>

/* Message exchange segment */
mppa_async_segment_t messages_segment;

/* Creates a message. */
struct message *message_create(int type, ...) {
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

#ifdef _MASTER_ /* MASTER SIDE */

/* Messages in progres */
struct message msgs_inProg[NUM_CLUSTERS];

/* Initializes message exchange context */
void create_message_segments() {
	createSegment(&messages_segment, MSG_SEG_ID, msgs_inProg, nclusters * sizeof(struct message), 0, 0, NULL);
}

#else /* SLAVE SIDE */

/* Be aware of message exchange context */
void clone_message_segments() {
	cloneSegment(&messages_segment, MSG_SEG_ID, 0, 0, NULL);
}

#endif

/* Destroys a message. */
void message_destroy(struct message *msg) {
	free(msg);
}

/* Sends a message. */
void message_put(struct message *msg) {
	dataPut(msg, &messages_segment, 0, 1, sizeof(struct message), NULL);
}

/* Receives a message. */
struct message *message_get(int offset) {
	size_t size = sizeof(struct message);
	struct message *msg;
	
	msg = message_create(DIE);
	
	dataGet(msg, &messages_segment, offset*size, 1, size, NULL);
	
	return (msg);
}