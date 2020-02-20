//---
//	gint:keysc:getkey - High-level keyboard monitoring function
//---

#include <gint/keyboard.h>
#include <gint/defs/types.h>

#ifdef FX9860G
#include <gint/drivers/t6k11.h>
#endif

/* Delay between a key press and the first repeat, in scan intervals */
static int rep_first = 64;
/* Delay between subsequent repeats, in scan intervals */
static int rep_next = 8;

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
	static int rep_key = 0;
	/* Number of repeats already emitted */
	static int rep_count = 0;
	/* Keyboard time when the key was pressed */
	static int rep_time = 0;

	/* Reset the state if the repeated key went up due to different
	   graphical primitives being used */
	if(rep_key && !keydown(rep_key))
	{
		rep_key = 0;
		rep_count = 0;
		rep_time = 0;
	}

	while(1) switch((ev = pollevent()).type)
	{
	/* Key press: handle modifiers or return an event */
	case KEYEV_DOWN:
		key = ev.key;
		if(rep_key && key != rep_key) break;

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
		rep_key = key;
		rep_count = 0;
		rep_time = ev.time;

		ev.mod = 1;
		ev.shift = shift;
		ev.alpha = alpha;
		return ev;

	/* If nothing happens, stop or wait for a repeat to occur */
	case KEYEV_NONE:
		/* Timeout has expired, return KEYEV_NONE */
		if(timeout && *timeout) return ev;

		/* Check that the last pressed key can be repeated */
		int arrow = (rep_key == KEY_LEFT || rep_key == KEY_RIGHT ||
		             rep_key == KEY_UP   || rep_key == KEY_DOWN);

		if(!rep_key || !(
			(opt & GETKEY_REP_ALL) ||
			(opt & GETKEY_REP_ARROWS && arrow)
		)) break;

		/* If the key is key pressed long enough, create a new event */
		int duration = (int16_t)(ev.time - rep_time);

		int target = (rep_count) ? rep_next : rep_first;
		if(duration < target) break;

		rep_time += target;
		rep_count++;

		ev.mod = 1;
		ev.shift = shift;
		ev.alpha = alpha;
		ev.type = KEYEV_HOLD;
		ev.key = rep_key;
		return ev;

	/* Reset repeating information if the repeated key is released */
	case KEYEV_UP:
		if(ev.key != rep_key) break;

		rep_key = 0;
		rep_count = 0;
		rep_time = 0;
		break;
	}
}

/* getkey_repeat() - set repeat delays for getkey() */
void getkey_repeat(int first, int next)
{
	rep_first = (first * KEYBOARD_SCAN_FREQUENCY) / 1000;
	rep_next = (next * KEYBOARD_SCAN_FREQUENCY) / 1000;
}
