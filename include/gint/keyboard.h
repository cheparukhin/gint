//---
//	gint:keyboard - Keyboard input
//---

#ifndef GINT_KEYBOARD
#define GINT_KEYBOARD

/* Keyboard, key events and getkey()

   gint's keyboard driver regularly scans the keyboard matrix and produces *key
   events*, the most primitive type of event exposed to the user. A key event
   basically says that a key has been pressed, held, or released at some point
   in time. They are suited for real-time applications like games.

   - pollevent() fetches the next queued keyboard event, and returns an event
     of type KEYEV_NONE if none is available.
   - waitevent() fetches the next queued keyboard event and waits if none is
     available. The timeout can be configured.

   GUI programs like the system applications will prefer using a GetKey()-like
   functions that return a single key press at a time, heeds for releases, for
   SHIFT and ALPHA modifiers, handles backlight and return-to-menu.

   - getkey_opt() is gint's enhanced GetKey()-like function, with support for
     custom repetition and many fine-tunable options.
   - getkey() is a specific call to getkey_opt(), that imitates GetKey(). */

#include <gint/defs/types.h>
#include <gint/keycodes.h>

/* key_event_t - any keyboard event
   This structure represents an event that occurs on the keyboard. This is a
   low-level structure that is produced by the keyboard scanner. It reports key
   presses, key releases, and key repeats.

   These events are detected and reported each time the keyboard is scanned,
   which is 128 Hz by default, so you'll get 128 repeat events by second if a
   key is kept pressed. We could filter the events to emit one only every
   second, for example, but it's difficult to do it for all keys at the same
   time. Thus the control of repeat delays is left to getkey().

   When [mod = 1], attributes [shift] and [alpha] indicate whether the key has
   been modified. Only key press events returned by getkey() have [mod = 1].
   Note that you can't have, e.g. [key=KEY_SHIFT] and [mod=1] at the same time.

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

/* Keyboard frequency analysis, must be at least 64 for the keyboard to work,
   and at most 32768 for the extra timer to support it. Better if a power of 2.
   TODO: Add a configure or runtime setting for KEYBOARD_SCAN_FREQUENCY */
#ifndef KEYBOARD_SCAN_FREQUENCY
#define KEYBOARD_SCAN_FREQUENCY 128
#endif

//---
//	Keyboard functions
//---

/* pollevent() - poll the next keyboard event
   This function returns the next yet-unpolled event from the keyboard buffer.
   If no event is available, it returns a dummy event with type=KEYEV_NONE.
   This function always returns events with mod=0. */
key_event_t pollevent(void);

/* waitevent() - wait for the next keyboard event
   This function works as pollevent() but waits if no event is available. When
   timeout=NULL, it waits indefinitely. Otherwise, it waits until *timeout
   becomes non-zero. It is particularly suitable to set *timeout to 1 using a
   timer with [timer_timeout] as callback. See <gint/timer.h>. */
key_event_t waitevent(volatile int *timeout);

/* getkey() - wait for a pressed key

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

	/* Default settings of getkey() */
	GETKEY_DEFAULT     = 0x1f,
};

/* getkey_opt() - enhanced getkey()

   This function enhances getkey() with more general features. An
   or-combination of option flags (see above) must be supplied as first
   argument; 0 stands for no option. getkey_opt() returns the same kind of
   values as getkey().

   getkey_opt() supports a generic timeout function in the form of a volatile
   pointer [timeout]. If it's NULL, getkey_opt() waits indefinitely. Otherwise,
   it waits until *timeout becomes non-zero. It's up to you to change the
   value whenever you want to interrupt the call; using a timer with
   [timer_timeout] as callback is suitable. See <gint/timer.h>.

   @options  An or-combination of values from the GETKEY_* enumeration
   @timeout  Optional pointer to a timeout value
   Returns a key event of type KEYEV_DOWN or KEYEV_HOLD with [mod=1]. */
key_event_t getkey_opt(int options, volatile int *timeout);

/* getkey_repeat() - set repeat delays for getkey()

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
