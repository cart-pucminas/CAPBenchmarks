/* Kernel Include */
#include <async_util.h>
#include <message.h>
#include <global.h>
#include <util.h>

/* C And MPPA Library Includes*/
#include <stdarg.h>

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
		msg->u.findwork.height = va_arg(ap, int);
		msg->type = FINDWORK;
		break;
		
		/* FINDRESULT. */
		case FINDRESULT :
		msg->u.findresult.ipvt = va_arg(ap, int);
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

		/* Statistics information. */
		case STATISTICSINFO :
		msg->u.info.data_put = va_arg(ap, size_t);
		msg->u.info.data_get = va_arg(ap, size_t);
		msg->u.info.nput = va_arg(ap, unsigned);
		msg->u.info.nget = va_arg(ap, unsigned);
		msg->u.info.total = va_arg(ap, uint64_t);
		msg->u.info.communication = va_arg(ap, uint64_t);
		msg->u.info.msgsignal = 0;
		break;

		/* Signal to proceed/end message. */
		default :
		msg->type = DIE;
		break;
	}
	
	msg->next = NULL;
	va_end(ap);
	
	return (msg);
}

/* Destroys a message. */
void message_destroy(struct message *msg) {
	free(msg);
}

/* Sends a message. */
void message_put(struct message *msg, mppa_async_segment_t *seg, int offset, mppa_async_event_t *event) {
	dataPut(msg, seg, offset, 1, sizeof(struct message), event);
}

/* Receives a message. */
struct message *message_get(mppa_async_segment_t *seg, int offset, mppa_async_event_t *event) {
	struct message *msg;

	msg = message_create(DIE);

	dataGet(msg, seg, offset, 1, sizeof(struct message), event);

	return msg;
}