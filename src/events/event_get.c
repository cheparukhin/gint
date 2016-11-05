#include <internals/events.h>
#include <events.h>
#include <clock.h>

/*
	pollevent()
	Returns the next event. If no one is available, returns an event whose
	type is ET_None. This function always returns immediately.
*/
struct Event pollevent(void)
{
	struct Event event = {
		.type	= ET_None
	};
	if(queue_size <= 0) return event;

	event = event_queue[queue_start];

	queue_size--;
	if(queue_start == EVENTS_QUEUE_SIZE - 1) queue_start = 0;
	else queue_start++;

	return event;
}

/*
	getevent()
	Returns the next event. If no one is available, waits for something to
	happen. This function uses low-level sleep and should be preferred to
	active waiting using loops.
*/
struct Event getevent(void)
{
	struct Event event;

	while((event = pollevent()).type == ET_None) sleep();
	return event;
}
