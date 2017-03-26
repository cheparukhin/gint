#include <internals/events.h>
#include <events.h>

volatile event_t event_queue[EVENTS_QUEUE_SIZE];
volatile int queue_start = 0;
volatile int queue_size = 0;

/*
	event_push()
	Queues a user-defined event, allowing it to be retrieved by getevent()
	or pollevent() later. Pushing ET_None events is not allowed.
	Returns non-zero on error.
*/
int event_push(event_t event)
{
	if(queue_size >= EVENTS_QUEUE_SIZE) return 1;
	if(event.type == event_none) return 2;

	int index = queue_start + queue_size;
	if(index >= EVENTS_QUEUE_SIZE) index -= EVENTS_QUEUE_SIZE;

	event_queue[index] = event;
	queue_size++;

	return 0;
}
