//---
//	gint:keysc - The SH7305 and I/O Key Scan Interfaces
//---

#include <gint/drivers.h>
#include <gint/gint.h>
#include <gint/timer.h>
#include <gint/clock.h>
#include <gint/keyboard.h>

#include <gint/defs/attributes.h>
#include <gint/defs/types.h>
#include <gint/drivers/iokbd.h>
#include <gint/hardware.h>

#include <stdarg.h>

//---
//	Keyboard buffer
//---

/* The driver's internal state. At each step of time, this file compares the
   internal state with the hardware state and generates events accordingly.
   Events can be seen as a delta-encoding of the keyboard state over time.

   To ensure that adding pending events to the last-read state always gives the
   internal driver state, this array is not updated if the generation of an
   event fails. (Most likely the even will be regenerated at the next scan.) */
GDATA static volatile uint8_t state[12] = { 0 };

/* The driver's current event state. This state corresponds to the sum of all
   events sent to the user so far.When the event queue is empty, this is equal
   to [state]. For each generated event, this array is updated to reflect the
   user's view of the keyboard. */
GDATA static uint8_t current[12] = { 0 };

/* A driver event, which is a common change in a full row */
typedef struct
{
	uint16_t time;    /* Locally unique time identifier */
	uint8_t row;      /* Row number */
	uint8_t changed;  /* Keys that changed state */
	uint8_t kind;     /* Type of change, either KEYEV_DOWN or KEYEV_UP */

} driver_event_t;

/* The keyboard event buffer. This is a circular list defined by [buffer_start]
   and [buffer_end]. To avoid an ambiguity when start == end, there must always
   be at least one free entry. */
GBSS static driver_event_t buffer[KEYBOARD_QUEUE_SIZE];
/* Buffer bounds */
GDATA static int buffer_start = 0;
GDATA static int buffer_end = 0;

/* Current time, in keyboard-scanning ticks */
GDATA static uint time = 0;

/* buffer_push(): Add an event in the keyboard buffer
   Returns non-zero if the event cannot be pushed. */
GMAPPED static int buffer_push(driver_event_t ev)
{
	int next = (buffer_end + 1) % KEYBOARD_QUEUE_SIZE;
	if(next == buffer_start) return 1;

	buffer[buffer_end] = ev;
	buffer_end = next;
	return 0;
}

/* buffer_poll(): Generate key events from the buffer
   Sets [ev] and returns zero on success, otherwise non-zero. */
static int buffer_poll(driver_event_t *ev)
{
	if(buffer_start == buffer_end) return 1;

	*ev = buffer[buffer_start];
	buffer_start = (buffer_start + 1) % KEYBOARD_QUEUE_SIZE;
	return 0;
}

/* keysc_frame(): Generate driver events from KEYSC state */
GMAPPED static void keysc_frame(void)
{
	GALIGNED(2) uint8_t scan[12] = { 0 };

	/* First scan the key matrix: from I/O ports on SH3, KEYSC on SH4 */
	if(isSH3()) iokbd_scan(scan);
	else
	{
		volatile uint16_t *KEYSC = (void *)0xa44b0000;
		uint16_t *array = (void *)&scan;

		for(int i = 0; i < 6; i++) array[i] = KEYSC[i];
	}

	/* Compare new data with the internal state. Push releases before
	   presses so that a key change occurring within a single analysis
	   frame can be performed. This happens all the time when going back to
	   the main MENU via gint_osmenu() on a keybind. */
	for(int row = 0; row < 12; row++)
	{
		int diff = ~scan[row] & state[row];
		if(!diff) continue;

		/* Update internal status if the event could be pushed */
		driver_event_t ev = { time, row, diff, KEYEV_UP };
		if(!buffer_push(ev)) state[row] &= scan[row];
	}
	for(int row = 0; row < 12; row++)
	{
		int diff = scan[row] & ~state[row];
		if(!diff) continue;

		driver_event_t ev = { time, row, diff, KEYEV_DOWN };
		if(!buffer_push(ev)) state[row] |= scan[row];
	}
}

/* pollevent() - poll the next keyboard event */
key_event_t pollevent(void)
{
	/* Every time a driver event is unqueued, its key events are generated
	   and put in this small buffer */
	static key_event_t events[8];
	/* Number of pending events in the previous buffer */
	static int events_pending = 0;

	/* Use pending events first, if they exist */

	if(events_pending > 0)
	{
		key_event_t ev = events[--events_pending];

		/* Update the current state buffer according to [ev] */
		int row = (ev.key >> 4) ^ 1;
		int col = 0x80 >> (ev.key & 0x7);

		if(ev.type == KEYEV_DOWN) current[row] |=  col;
		if(ev.type == KEYEV_UP)   current[row] &= ~col;

		return ev;
	}

	/* If not key event is pending, generate a chunk by polling the driver
	   event queue */

	driver_event_t ev;
	if(buffer_poll(&ev))
	{
		key_event_t ev = { .type = KEYEV_NONE, .time = time };
		return ev;
	}

	/* Generate new key events and return the first of them*/
	int changed = ev.changed;
	for(int code = ((ev.row ^ 1) << 4) | 0x7; code & 0x7; code--)
	{
		if(changed & 1)
		{
			key_event_t keyev = {
				.time = ev.time,
				.type = ev.kind,
				.key = code,
			};
			events[events_pending++] = keyev;
		}
		changed >>= 1;
	}

	/* Call recursively to avoid duplicating the event emission code */
	return pollevent();
}

/* waitevent() - wait for the next keyboard event */
key_event_t waitevent(volatile int *timeout)
{
	while(1)
	{
		key_event_t ev = pollevent();
		if(ev.type != KEYEV_NONE) return ev;

		if(timeout && *timeout) break;
		sleep();
	}

	key_event_t ev = { .type = KEYEV_NONE, .time = time };
	return ev;
}

/* clearevents(): Read all events waiting in the queue */
void clearevents(void)
{
	while(pollevent().type != KEYEV_NONE);
}

//---
//	Immediate key access
//---

/* keydown(): Current key state */
int keydown(int key)
{
	int row = (key >> 4) ^ 1;
	int col = 0x80 >> (key & 0x7);

	return (current[row] & col) != 0;
}

/* keydown_all(): Check a set of keys for simultaneous input
   Returns non-zero if all provided keys are down. The list should end with an
   integer 0 as terminator. */
int keydown_all(int key, ...)
{
	va_list args;
	va_start(args, key);

	int st = 1;
	while(key && st)
	{
		st = keydown(key);
		key = va_arg(args, int);
	}

	va_end(args);
	return st;
}

/* keydown_any(): Check a set of keys for any input
   Returns nonzero if any one of the specified keys is currently pressed. THe
   sequence should be terminated by a 0 integer. */
int keydown_any(int key, ...)
{
	va_list args;
	va_start(args, key);

	int st = 0;
	while(key && !st)
	{
		st = keydown(key);
		key = va_arg(args, int);
	}

	va_end(args);
	return st;
}

//---
//	Driver initialization
//---

GMAPPED static int callback(GUNUSED volatile void *arg)
{
	keysc_frame();
	time++;
	return 0;
}

/* init() - setup the support timer */
static void init(void)
{
	int tid = isSH3() ? 3 : 8;

	/* Configure the timer to do 128 keyboard scans per second. This
	   frequency *must* be high for the KEYSC interface to work! */
	/* Note: the supporting timer always runs at 32768 Hz. */
	int delay = 32768 / KEYBOARD_SCAN_FREQUENCY;
	if(!delay) delay = 1;

	/* Set the default repeat times (milliseconds) */
	getkey_repeat(400, 40);

	/* The timer will be stopped when the timer driver is unloaded */
	timer_setup(tid, delay, 0, callback, NULL);
	timer_start(tid);

	gint[HWKBD] = HW_LOADED | (isSH3() ? HWKBD_IO : HWKBD_KSI);
	gint[HWKBDSF] = KEYBOARD_SCAN_FREQUENCY;
}

//---
//	Driver structure definition
//---

gint_driver_t drv_keysc = {
	.name    = "KEYSC",
	.init    = init,
};

GINT_DECLARE_DRIVER(4, drv_keysc);
