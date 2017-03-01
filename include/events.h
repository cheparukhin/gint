//---
//
//	gint core module: events
//
//	Finally some user-friendly API.
//
//---

#ifndef	_EVENTS_H
#define	_EVENTS_H

#include <timer.h>

/*
	event_type_t
	Something user programs will surely use most often.
*/
typedef enum
{
	EventType_None			= 0,
	ET_None				= EventType_None,

	EventType_User			= 1,
	ET_User				= EventType_User,

	EventType_KeyPressed		= 2,
	ET_KeyPress			= EventType_KeyPressed,

	EventType_KeyRepeated		= 3,
	ET_KeyRepeat			= EventType_KeyRepeated,

	EventType_KeyReleased		= 4,
	ET_KeyRel			= EventType_KeyReleased,

	EventType_TimerUnderflow	= 5,
	ET_Timer			= EventType_TimerUnderflow,

} event_type_t;

/*
	event_t
	Wake up, something's going on. The union member that holds information
	about the event is implicitly defined by the type attribute.
*/
typedef struct
{
	event_type_t type;

	union
	{
		// For ET_User.
		void *data;
		// For ET_KeyPress, ET_KeyRepeat and ET_KeyRel.
		int key;
		// For ET_Timer.
		timer_t *timer;
	};

} event_t;



//---
//	Event management.
//---

/*
	event_push()
	Queues a user-defined event, allowing it to be retrieved by getevent()
	or pollevent() later. Most often you will not need to use this, as
	system events are automatically queued. Pushing ET_None events is not
	allowed.
	Returns non-zero on error.
*/
int event_push(event_t event);

/*
	waitevent()
	Returns the next event. If no one is available, waits for something to
	happen. This function uses low-level sleep and should be preferred to
	active waiting using loops.
*/
event_t waitevent(void);

/*
	pollevent()
	Returns the next event. If no one is available, returns an event whose
	type is ET_None. This function always returns immediately.
*/
event_t pollevent(void);

#endif	// _EVENTS_H
