//---
//	gint:keysc - The SH7305 and I/O Key Scan Interfaces
//---

#include <gint/drivers.h>
#include <gint/gint.h>
#include <gint/timer.h>
#include <gint/clock.h>
#include <gint/keyboard.h>

#include <gint/drivers/iokbd.h>
#include <gint/defs/attributes.h>
#include <gint/hardware.h>

//---
//	Keyboard buffer
//---

/* The driver's internal state. At each step of time, this file compares the
   internal state with the hardware state and generates events accordingly.
   Events can be seen as a delta-encoding of the keyboard state over time.

   The user which sums up these events to maintain a full keyboard state must
   get a correct result. As a consequence, if an event cannot be generated
   (whatever the reason), the driver's internal copy of the keyboard state must
   not be updated (probably the event will be re-emitted at the next scan). */
GDATA static volatile uint8_t state[12] = { 0 };

/* A driver event, which is a change in a full row instead of a single key. */
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
GDATA static int time = 0;

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

/* keysc_frame() - generate a round of events for the current frame */
static void keysc_frame(void)
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

/* pollevent() - poll the next keyboard event */
key_event_t pollevent(void)
{
	/* Every time a driver event is unqueued, its key events are generated
	   and put in this small buffer */
	static key_event_t events[8];
	/* Number of pending events in the previous buffer */
	static int events_pending = 0;

	/* Use pending events first, then poll the driver buffer */

	if(events_pending > 0) return events[--events_pending];

	driver_event_t ev;
	if(buffer_poll(&ev)) return (key_event_t){ .type = KEYEV_NONE };

	/* Generate new key events and return the first of them*/
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

	return events[--events_pending];
}

/* waitevent() - wait for the next keyboard event */
key_event_t waitevent(volatile int *timeout)
{
	key_event_t none = { .type = KEYEV_NONE };

	while(1)
	{
		key_event_t ev = pollevent();
		if(ev.type != KEYEV_NONE) return ev;

		if(timeout && *timeout) break;
		sleep();
	}

	return none;
}

//---
//	Driver initialization and status
//---

static int callback(GUNUSED volatile void *arg)
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
	/* TODO: The SH3 does not need to run fast, adjust user settings? */
	int delay = 32768 / KEYBOARD_SCAN_FREQUENCY;
	if(!delay) delay = 1;

	/* Set the default repeat times (milliseconds) */
	getkey_repeat(500, 125);

	timer_setup(tid, delay, 0, callback, NULL);
	timer_start(tid);

	gint[HWKBD] = HW_LOADED | (isSH3() ? HWKBD_IO : HWKBD_KSI);
	gint[HWKBDSF] = KEYBOARD_SCAN_FREQUENCY;
}

/* unload() - stop the support timer */
static void unload(void)
{
	int tid = isSH3() ? 3 : 8;
	timer_stop(tid);
}

#ifdef GINT_BOOT_LOG

/* keysc_status() - status string of the driver */
static const char *keysc_status(void)
{
	static char str[3] = "Sw";
	if(isSH3()) str[0] = 's';
	return str;
}

#endif /* GINT_BOOT_LOG */

//---
//	Driver structure definition
//---

gint_driver_t drv_keysc = {
	.name		= "KEYSC",
	.init		= init,
	.status		= GINT_DRIVER_STATUS(keysc_status),
	.unload		= unload,
	.ctx_size	= 0,
	.sys_ctx	= NULL,
	.ctx_save	= NULL,
	.ctx_restore	= NULL,
};

GINT_DECLARE_DRIVER(4, drv_keysc);
