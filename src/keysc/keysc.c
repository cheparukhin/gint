//---
//	gint:keysc - The SH7305 and I/O Key Scan Interfaces
//---

#include <gint/drivers.h>
#include <gint/gint.h>
#include <gint/timer.h>
#include <gint/clock.h>
#include <gint/keyboard.h>
#include <gint/drivers/keydev.h>

#include <gint/defs/attributes.h>
#include <gint/defs/types.h>
#include <gint/drivers/iokbd.h>
#include <gint/hardware.h>

#include <stdarg.h>

/* Keyboard scan frequency in Hertz. Start with 128 Hz, this frequency *must
   be high* for the keyboard to work! Reading at low frequencies produces a lot
   of artifacts. See https://www.casiopeia.net/forum/viewtopic.php?p=20592. */
static int scan_frequency = 128;
/* Approximation in microseconds, used by the timer and repeat delays */
static uint32_t scan_frequency_us = 7812; /* 1000000 / scan_frequency */

/* Keyboard scanner timer */
static int keysc_tid = -1;
/* Keyboard input device for this Key Scan Interface */
static keydev_t dev_keysc;

/* keydev_std(): Standard keyboard input device */
keydev_t *keydev_std(void)
{
	return &dev_keysc;
}

/* keysc_scan_frequency(): Get the current keyboard scan frequency in Hertz */
int keysc_scan_frequency(void)
{
	return scan_frequency;
}

/* keysc_scan_frequency_us(): Get keyboard scan delay in microseconds */
uint32_t keysc_scan_frequency_us(void)
{
	return scan_frequency_us;
}

/* keysc_set_scan_frequency(): Set the keyboard scan frequency in Hertz */
void keysc_set_scan_frequency(int freq)
{
	if(freq < 64) freq = 64;
	if(freq > 32768) freq = 32768;
	scan_frequency = freq;
	scan_frequency_us = 1000000 / freq;

	if(keysc_tid < 0) return;
	uint32_t TCOR = timer_delay(keysc_tid, scan_frequency_us, 0);
	timer_reload(keysc_tid, TCOR);
}

/* keysc_tick(): Update the keyboard to the next state */
static int keysc_tick(void)
{
	GALIGNED(2) uint8_t scan[12] = { 0 };

	/* Scan the key matrix: from I/O ports on SH3, KEYSC on SH4 */
	if(isSH3()) iokbd_scan(scan);
	else
	{
		volatile uint16_t *KEYSC = (void *)0xa44b0000;
		uint16_t *array = (void *)&scan;

		for(int i = 0; i < 6; i++) array[i] = KEYSC[i];
	}

	keydev_process_state(&dev_keysc, scan);
	keydev_tick(&dev_keysc, scan_frequency_us);
	return TIMER_CONTINUE;
}

/* pollevent() - poll the next keyboard event */
key_event_t pollevent(void)
{
	return keydev_unqueue_event(&dev_keysc);
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

	key_event_t ev = { .type = KEYEV_NONE, .time = dev_keysc.time };
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
	return keydev_keydown(&dev_keysc, key);
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

static void configure(void)
{
	keydev_init(&dev_keysc);

	/* Set the default repeat times (milliseconds) */
	getkey_repeat(400, 40);

	/* The timer will be stopped when the timer driver is unloaded */
	keysc_tid = timer_setup(TIMER_ANY, scan_frequency_us, keysc_tick);
	if(keysc_tid >= 0) timer_start(keysc_tid);

	gint[HWKBD] = HW_LOADED | (isSH3() ? HWKBD_IO : HWKBD_KSI);
}

//---
// State and driver metadata
//---

gint_driver_t drv_keysc = {
	.name       = "KEYSC",
	.configure  = configure,
};
GINT_DECLARE_DRIVER(23, drv_keysc);
