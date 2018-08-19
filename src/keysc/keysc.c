//---
//	gint:keysc - The SH7305 and I/O Key Scan Interfaces
//---

#include <gint/drivers.h>
#include <gint/gint.h>
#include <gint/timer.h>
#include <gint/keyboard.h>

#include <core/mpu.h>
#include <defs/attributes.h>

//---
//	Keyboard buffer
//---

/* The driver's internal state. At each step of time, this file compares the
   internal state with the hardware state and generates events accordingly.
   Events can be seen as a delta-encoding of the state of the keyboard over
   time, and this buffer must be the sum of all events. This means that if an
   event cannot be generated, f.i. because the buffer is full, this state must
   *not* be updated. */
GDATA volatile uint8_t state[12] = { 0 };

/* A driver event, which is a change in a full row, not a key. */
typedef struct
{
	uint time	:12;	/* Locally unique time identifier */
	uint row	:4;	/* Row number */
	uint old	:8;	/* Key status for the old row */
	uint new	:8;	/* Key status for the new row */

} driver_event_t;

/* The keyboard event buffer. This is a circular list defined by [buffer_start]
   and [buffer_end]. To avoid an ambiguity when start = end, the buffer is not
   allowed to be full (at least one free cell must be remaining). */
GBSS static driver_event_t buffer[KEYBOARD_QUEUE_SIZE];
/* Buffer bounds */
GDATA static int buffer_start = 0;
GDATA static int buffer_end = 0;

/* Current time, in keyboard-scanning ticks */
GDATA int time = 0;
GDATA int full_release = 2;

/* buffer_push() - add an event in the keyboard buffer
   Returns non-zero if the event cannot be pushed. */
static int buffer_push(driver_event_t ev)
{
	int next = (buffer_end + 1) % KEYBOARD_QUEUE_SIZE;
	if(next == buffer_start) return 1;

	buffer[buffer_end] = ev;
	buffer_end = next;
	return 0;
}

/* buffer_poll() - generate key events from the buffer
   Sets [ev] and returns zero on success, otherwise non-zero. */
static int buffer_poll(driver_event_t *ev)
{
	if(buffer_start == buffer_end) return 1;

	*ev = buffer[buffer_start];
	buffer_start = (buffer_start + 1) % KEYBOARD_QUEUE_SIZE;
	return 0;
}


/* keysc_frame() - generate a round of interrupts for the current frame */
void keysc_frame(void)
{
	ALIGNED(2) uint8_t scan[12] = { 0 };
	time++;

	/* First scan the key matrix: from I/O ports on SH3, KEYSC on SH4 */
	if(isSH3()) iokbd_scan(&scan);
	else
	{
		volatile uint16_t *KEYSC = (void *)0xa44b0000;
		uint16_t *array = (void *)&scan;

		for(int i = 0; i < 6; i++) array[i] = KEYSC[i];
	}

	for(int row = 0; row < 12; row++)
	{
		/* Compare new data with the internal state. */
		int old = state[row];
		int new = scan[row];
		if(old == new && !new) continue;

		driver_event_t ev = {
			.time = time,
			.row  = row,
			.old  = old,
			.new  = new,
		};

		/* Update internal status if the event could be pushed */
		if(!buffer_push(ev)) state[row] = new;
	}
}

/* key_poll() - poll the next keyboard event from the buffer */
key_event_t key_poll(void)
{
	/* Every time a driver event is unqueued, its key events are generated
	   and put in this small buffer */
	static key_event_t events[8];
	/* Number of pending events in the previous buffer */
	static int events_pending = 0;

	/* If there are pending events, use them */
	if(events_pending > 0) return events[--events_pending];

	/* Unqueue something from the driver buffer, if possible */
	driver_event_t ev;
	if(buffer_poll(&ev)) return (key_event_t){ .type = KEYEV_NONE };

	/* Generate new key events */
	int old = ev.old << 1;
	int new = ev.new;

	for(int code = ((ev.row ^ 1) << 4) | 0x7; code & 0x7; code--)
	{
		int kind = (old & 2) | (new & 1);
		old >>= 1;
		new >>= 1;

		if(!kind) continue;

		key_event_t keyev = {
			.time	= ev.time,
			.type	= kind,
			.key	= code,
		};
		events[events_pending++] = keyev;
	}

	/* Return the first of these generated events */
	return events[--events_pending];
}

//---
//	Driver initialization
//---

static int callback(UNUSED void *arg)
{
	keysc_frame();
	return 0;
}

/* init() - setup the support timer */
static void init(void)
{
	int tid = isSH3() ? 4 : 7;

	/* 32768 Hz divided by 16 is 2048 */
	/* TODO: Use I/O port scanning on SH3 */
	timer_setup(tid, 2048, 0, callback, NULL);
	timer_start(tid);
}

/* unload() - stop the support timer */
static void unload(void)
{
	int tid = isSH3() ? 4 : 7;
	timer_stop(tid);
}

//---
//	Driver structure definition
//---

gint_driver_t drv_keysc = {
	.name		= "KEYSC",
	.init		= init,
	.unload		= unload,
	.ctx_size	= 0,
	.sys_ctx	= NULL,
	.ctx_save	= NULL,
	.ctx_restore	= NULL,
};

GINT_DECLARE_DRIVER(4, drv_keysc);
