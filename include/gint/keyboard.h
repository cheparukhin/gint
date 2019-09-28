//---
//	gint:keyboard - Keyboard input
//---

#ifndef GINT_KEYBOARD
#define GINT_KEYBOARD

/* Keyboard, key events, keydown() and getkey()

   gint's keyboard driver regularly scans the keyboard matrix and produces *key
   events*. A key event basically says that a key has been pressed or released
   at some point in time. Key events are a faithful description of everything
   that happens on the keyboard.

   These key events are stored in the *event queue*, from where they can be
   retrieved:

   * pollevent() fetches the next keyboard event waiting in the queue. Events
     will always be returned in chronological order. If there is no keyboard
     event pending (ie. if nothing happened since last time events were read),
     this function returns a dummy event of type KEYEV_NONE.
   * waitevent() fetches the next keyboard event waiting in the queue. If the
     queue is empty, it waits until one becomes available or a timeout expires,
     whichever comes first. The timeout can be configured.

   When events have been read, other functions can be read to check whether a
   certain key (or set of keys) is pressed.

   * keydown() checks if the specified key is currently pressed. (*)
   * keydown_all() checks if all keys of a specified list are pressed. The list
     should end with value 0.
   * keydown_any() checks if any key of a specified list is pressed. The list
     should end with value 0.

   (*) The keydown() functions do not report the absolute current state of the
       key, but rather the state of the key according to the events that have
       been retrieved so far. If the queue contains {KEY_A, KEYEV_DOWN} and
       {KEY_A, KEYEV_UP}, the following sequence of events will happen:

         keydown(KEY_A)   -> 0
         pollevent()      -> {KEY_A, KEYEV_DOWN}
         keydown(KEY_A)   -> 1
         pollevent()      -> {KEY_A, KEYEV_UP}
         keydown(KEY_A)   -> 0

       Synchronizing keydown() and the events increases the accuracy of
       keyboard information for fast programs and its reliability for slow
       programs.

       When all events have been read from the queue, keydown() returns the
       absolute current key state, which is what will happen 90% of the time.

   Applications that are not interested in event contents but only in pressed
   keys can automatically read all events from the queue before they start
   using keydown():

   * clearevents() reads all pending events from the input queue.

   The previous functions are quite low-level. GUI programs that look like the
   system applications will prefer using a GetKey()-like functions that return
   a single key press at a time, heeds for releases, for SHIFT and ALPHA
   modifiers, handles repeats, backlight and return-to-menu.

   * getkey_opt() is gint's enhanced GetKey()-like function, with support for
     custom repetition and many fine-tunable options.
   * getkey() is a specific call to getkey_opt(), that imitates GetKey().

   These functions introduce a new type of key event called KEYEV_HOLD which
   represents a key repeat. */

#include <gint/defs/types.h>
#include <gint/keycodes.h>

/* key_event_t: Low-level or high-level keyboard event

   This structure represents an event that occurs on the keyboard. It is first
   produced by the keyboard scanner with limited information, then possibly
   enriched by getkey(). Events are produced each time the keyboard is scanned,
   which is 128 Hz by default. Hence, a key press and release occuring in less
   than 8 ms might not be detected.

   getkey() returns enriched events with [mod=1], in whic ase [shift] and
   [alpha] indicate whether the key has been modified. Only key press events
   returned by getkey() have [mod=1]. Note that you can't have, e.g.
   [key=KEY_SHIFT] and [mod=1] at the same time.

   The [time] attribute indicates when the event occurred. It is a snapshot of
   a time counter that increases at each keyboard scan and *wraps around every
   8 minutes* (at 128 Hz). I expect this attribute to be useful to analyze
   combo sequences in games. Make sure you are aware of the two nitpicks:
   * Don't keep the time values for too long because of the wrap-around effect.
   * 0xffff is "just before" 0x0000, not "long after". */
typedef struct
{
	uint time	:16;	/* Time of event, unique over short periods */

	uint		:3;	/* Reserved for future use */

	uint mod	:1;	/* Whether modifiers are used */
	uint shift	:1;	/* If mod=1, whether SHIFT was pressed */
	uint alpha	:1;	/* If mod=1, whether ALPHA was pressed */

	uint type	:2;	/* Type of key event */
	uint key	:8;	/* Hit key */

} GPACKED(4) key_event_t;

/* Keyboard event types, as in the [type] field of key_event_t */
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

/* Keyboard frequency analysis, must be at least 64 for the keyboard to work,
   and at most 32768 for the extra timer to support it. Better if a power of 2.
   TODO: Add a configure or runtime setting for KEYBOARD_SCAN_FREQUENCY */
#ifndef KEYBOARD_SCAN_FREQUENCY
#define KEYBOARD_SCAN_FREQUENCY 128
#endif

//---
//	Event-level functions
//---

/* pollevent(): Poll the next keyboard event
   This function returns the next event from the event queue, chronologically.
   If no event is available, it returns a dummy event with type=KEYEV_NONE
   and time set to the current driver time. This function always returns events
   with mod=0. */
key_event_t pollevent(void);

/* waitevent(): Wait for the next keyboard event
   This function works as pollevent() but waits if no event is available. When
   timeout=NULL, it waits indefinitely. Otherwise, it waits until *timeout
   becomes non-zero. It is particularly suitable to set *timeout to 1 using a
   timer with [timer_timeout] as callback. See <gint/timer.h>. */
key_event_t waitevent(volatile int *timeout);

/* clearevents(): Read all events waiting in the queue */
void clearevents(void);

//---
//	Key state functions
//---

/* keydown(): Current key state
   This function returns zero if the specified key is currently up (according
   to the last events that have been processed) and non-zero if it is down. */
int keydown(int key);

/* keydown_all(): Check a set of keys for simultaneous input
   Returns non-zero if all provided keys are down. The list should end with a 0
   as terminator. */
int keydown_all(int key1, ...);

/* keydown_any(): Check a set of keys for any input
   Returns nonzero if any one of the specified keys is currently pressed. The
   sequence should be terminated by a 0 integer. */
int keydown_any(int key1, ...);

//---
//	High-level functions
//---

/* getkey(): Wait for a key press

   This function mimics the behavior of the fxlib GetKey(). It returns a
   key_event_t object where [mod=1], and where [shift] and [alpha] indicate
   whether SHIFT or ALPHA was pressed before the key was hit. [event] is
   KEYEV_DOWN when a new key is pressed and KEYEV_HOLD in case of repeats.

   Similarities with GetKey() include:
   - Wait for a key to be pressed *after* the call (held keys don't count)
   - Supports SHIFT and ALPHA modifiers
   - Repeats arrows keys
   - Allows return to main menu if the MENU key is pressed
   - Controls backlight on models that have a back-lit screen

   getkey() is equivalent to getkey_opt(GETKEY_DEFAULT, NULL). */
key_event_t getkey(void);

/* The following are the option bits for getkey_opt(). */
enum {
	/* Enable modifiers keys */
	GETKEY_MOD_SHIFT   = 0x01,
	GETKEY_MOD_ALPHA   = 0x02,
	/* SHIFT + OPTN toggles backlight (requires GETKEY_MOD_SHIFT) */
	GETKEY_BACKLIGHT   = 0x04,
	/* MENU triggers a task switch and displays the main menu */
	GETKEY_MENU        = 0x08,
	/* Repeat arrow keys, or even all keys */
	GETKEY_REP_ARROWS  = 0x10,
	GETKEY_REP_ALL     = 0x20,

	/* No modifiers */
	GETKEY_NONE        = 0x00,
	/* Default settings of getkey() */
	GETKEY_DEFAULT     = 0x1f,
};

/* getkey_opt(): Enhanced getkey()

   This function enhances getkey() with more general features. An
   or-combination of option flags (see above) must be supplied as first
   argument; GETKEY_NONE stands for no option. getkey_opt() returns the same
   kind of events as getkey().

   getkey_opt() supports a generic timeout function in the form of a volatile
   pointer [timeout]. If it's NULL, getkey_opt() waits indefinitely. Otherwise,
   it waits until *timeout becomes non-zero. It's up to you to change the
   value whenever you want to interrupt the call; using a timer with
   [timer_timeout] as callback is suitable. See <gint/timer.h>.

   @options  An or-combination of values from the GETKEY_* enumeration
   @timeout  Optional pointer to a timeout value
   Returns a key event of type KEYEV_DOWN or KEYEV_HOLD with [mod=1]. */
key_event_t getkey_opt(int options, volatile int *timeout);

/* getkey_repeat(): Set repeat delays for getkey()

   This function updates the repeat delays of getkey() and getkey_opt(). The
   unit of the argument is in milliseconds, but the granularity of the delay is
   dependent on the keyboard scan frequency.

   In the default setting (128 Hz scans), the possible repeat delays are
   approximately 8 ms, 16 ms, 23 ms, 31 ms... the provided arguments will be
   rounded to the closest feasible delays to ensure that repetitions are
   perfectly regular, rather than approximating the requested frequency.

   The system default is (500 ms, 125 ms). With the 128 Hz setting, this
   default is reached exactly without approximation.

   @first  Delay between key press and first repeat (no more than one hour)
   @next   Delay between subsequent repeats (no more than one hour) */
void getkey_repeat(int first, int next);

#endif /* GINT_KEYBOARD */
