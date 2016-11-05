//---
//
//	gint core module: events
//
//	Finally some user-friendly API.
//
//---

#ifndef	_EVENTS_H
#define	_EVENTS_H

//---
//	Type definitions.
//---

/*
	enum EventType
	Something user programs will surely use most often.
*/
enum EventType
{
	EventType_None		= 0,
	ET_None			= EventType_None,

	EventType_User		= 1,
	ET_User			= EventType_User,

	EventType_KeyPressed	= 2,
	ET_KeyPress		= EventType_KeyPressed,

	EventType_KeyReleased	= 3,
	ET_KeyRel		= EventType_KeyReleased,
};

/*
	struct Event
	Wake up, something's going on. The union member that holds information
	about the event is implicitly defined by the type attribute.
*/
struct Event
{
	enum EventType type;

	union
	{
		// For ET_User.
		void *data;
		// For ET_KeyPress and ET_KeyRel.
		int key;
	};
};



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
int event_push(struct Event event);

/*
	getevent()
	Returns the next event. If no one is available, waits for something to
	happen. This function uses low-level sleep and should be preferred to
	active waiting using loops.
*/
struct Event getevent(void);

/*
	pollevent()
	Returns the next event. If no one is available, returns an event whose
	type is ET_None. This function always returns immediately.
*/
struct Event pollevent(void);

#endif	// _EVENTS_H
