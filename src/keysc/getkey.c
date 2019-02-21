//---
//	gint:keysc:getkey - High-level keyboard monitoring function
//---

#include <gint/keyboard.h>
#include <gint/defs/types.h>

#ifdef FX9860G
#include <gint/drivers/t6k11.h>
#endif

/* Atom for counting time in getkey() is 32768 Hz, 1ull << ATOM is a second */
#define ATOM 15

/* Delay between a key press and the first repeat, in 1/32768 seconds */
static uint64_t rep_first;
/* Delay between subsequent repeats, in 1/32768 seconds */
static uint64_t rep_next;

/* getkey() - wait for a pressed key */
key_event_t getkey(void)
{
	return getkey_opt(GETKEY_DEFAULT, NULL);
}

/* getkey_opt() - enhanced getkey() */
key_event_t getkey_opt(int opt, volatile int *timeout)
{
	key_event_t ev;
	int shift = 0, alpha = 0, key = 0;

	/* Last pressed key (only this key may be repeated) */
	int rep_key = 0;
	/* Number of repeats already emitted */
	int rep_count = 0;
	/* Time elapsed since last repeat emission (in atoms) */
	uint64_t rep_time = 0;

	while(1) switch((ev = waitevent(timeout)).type)
	{
	/* Timeout has expired, return KEYEV_NONE */
	case KEYEV_NONE:
		return ev;

	/* Key press: handle modifiers or return an event */
	case KEYEV_DOWN:
		key = ev.key;

		/* Handle backlight on fx9860g */
		#ifdef FX9860G
		if(opt & GETKEY_BACKLIGHT && key == KEY_OPTN && shift)
		{
			t6k11_backlight(-1);
			shift = 0;
			continue;
		}
		#endif

		/* Return to menu. TODO: use gint_switch() in getkey_opt() */
		if(opt & GETKEY_MENU && key == KEY_MENU && !(alpha || shift))
		{
			continue;
		}

		/* Update modifiers */
		if(opt & GETKEY_MOD_SHIFT && key == KEY_SHIFT)
		{
			shift ^= 1;
			rep_key = 0;
			continue;
		}
		if(opt & GETKEY_MOD_ALPHA && key == KEY_ALPHA)
		{
			alpha ^= 1;
			rep_key = 0;
			continue;
		}

		/* Return current event */
		rep_key   = key;
		rep_count = 0;
		rep_time  = 0;

		ev.mod = 1;
		ev.shift = shift;
		ev.alpha = alpha;
		return ev;

	/* Return new events when a key is held (maybe) */
	case KEYEV_HOLD:
		if(ev.key != rep_key) break;
		rep_time += (1ull << ATOM) / KEYBOARD_SCAN_FREQUENCY;

		/* If the key is key pressed long enough, create a new event */
		if(rep_time < (rep_count ? rep_next : rep_first)) break;

		ev.mod = 1;
		ev.shift = shift;
		ev.alpha = alpha;
		return ev;

	/* Reset repeating information if the repeated key is released */
	case KEYEV_UP:
		if(ev.key != rep_key) break;

		rep_key   = 0;
		rep_count = 0;
		rep_time  = 0;
		break;
	}
}

/* getkey_repeat() - set repeat delays for getkey() */
void getkey_repeat(int first, int next)
{
	rep_first = ((uint64_t)first << ATOM) / 1000;
	rep_next = ((uint64_t)next << ATOM) / 1000;
}
