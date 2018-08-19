//---
//	gint:keyboard - Keyboard input
//---

#ifndef GINT_KEYBOARD
#define GINT_KEYBOARD

#include <defs/types.h>

/* key_event_t - any keyboard event
   This structure represents an event that occurs on the keyboard. This is a
   low-level structure that is produced by the keyboard scanner. It reports key
   presses, key releases, and key repetitions.

   These events are detected and reported each time the keyboard is scanned,
   which is 16 Hz by default, so you'll get 16 repeat events by second if a key
   is kept pressed. We can filter the events to emit one only every second, for
   example, but it's difficult to do it for all keys at the same time. Thus the
   control of repetition delays is restricted to getkey().

   When mod = 1, shift and alpha indicate whether the key has been modified.
   This is only possible for key press events returned by getkey(). Note that
   you can't have key = shift and mod = 1 at the same time.

   The time attribute indicates when the event occurred. This value increases
   at each keyboard scan and *it wraps around every 4 minutes* (at 16 Hz).
   I expect this attribute to be useful to analyze combo sequences in games.
   Make sure you are aware of the two nitpicks:
   - Don't keep the time values for too long because the wrap-around effect.
   - 0xfff is just before 0x000, not long after. */
typedef struct
{
	uint time	:12;	/* Time of event, unique over short periods */

	uint		:7;	/* Reserved for future use */

	uint mod	:1;	/* Whether modifiers are used */
	uint shift	:1;	/* If mod=1, whether SHIFT was pressed */
	uint alpha	:1;	/* If mod=1, whether ALPHA was pressed */

	uint type	:2;	/* Type of key event */
	uint key	:8;	/* Hit key */

} PACKED(4) key_event_t;

/* Keyboard event types, as in the type field of key_event_t */
enum
{
	KEYEV_NONE = 0,		/* No event available (poll() only) */
	KEYEV_DOWN = 1,		/* Key was pressed */
	KEYEV_UP   = 2,		/* Key was released */
	KEYEV_HOLD = 3,		/* A key that was pressed has been held down */
};

/* Size of the buffer event queue, can be customized using gint's configure
   script before compiling the library. Better be a power of 2. */
#ifndef KEYBOARD_QUEUE_SIZE
#define KEYBOARD_QUEUE_SIZE 64
#endif

//---
//	Keyboard functions
//---

key_event_t key_poll(void);

#endif /* GINT_KEYBOARD */
