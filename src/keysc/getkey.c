//---
//	gint:keysc:getkey - High-level keyboard monitoring function
//---

#include <gint/keyboard.h>
#include <gint/gint.h>
#include <gint/defs/types.h>

#ifdef FX9860G
#include <gint/drivers/t6k11.h>
#endif

/* Delay between a key press and the first repeat, in scan intervals */
static int rep_first = 64;
/* Delay between subsequent repeats, in scan intervals */
static int rep_next = 8;

/* Repeat filter function */
static int (*filter_function)(int key, int duration, int count) = NULL;

/* getkey(): Wait for a key press */
key_event_t getkey(void)
{
	return getkey_opt(GETKEY_DEFAULT, NULL);
}

/* getkey_opt(): Enhanced getkey() */
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
	/* Additional repeat delay set by the filtering function */
	static int rep_delay = 0;

	/* Reset the state if the repeated key went up while getkey() was not
	   aware (this happens when different keyboard primitives are used) */
	if(rep_key && !keydown(rep_key))
	{
		rep_key = 0;
		rep_count = 0;
		rep_time = 0;
		rep_delay = 0;
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

		/* Return to menu */
		if(opt & GETKEY_MENU && key == KEY_MENU && !(alpha || shift))
		{
			gint_osmenu();
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
		rep_delay = 0;

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

		if(rep_delay < 0) break;
		int target = (rep_count ? rep_next : rep_first) + rep_delay;
		if(duration < target) break;

		/* Filter out the event if repeat filtering is on */
		if(filter_function && (opt & GETKEY_REP_FILTER))
		{
			int s = filter_function(rep_key, duration, rep_count);
			/* Drop repeats forever */
			if(s < 0)
			{
				rep_delay = -1;
				break;
			}
			/* Delay repeat by set amount */
			if(s > 0)
			{
				s = (s * KEYBOARD_SCAN_FREQUENCY) / 1000;
				rep_delay += s;
				break;
			}
			/* Accepts repeat (fallthrough) */
		}

		rep_time += target;
		rep_count++;
		rep_delay = 0;

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
		rep_delay = 0;
		break;
	}
}

/* getkey_repeat(): Set repeat delays for getkey() */
void getkey_repeat(int first, int next)
{
	rep_first = (first * KEYBOARD_SCAN_FREQUENCY) / 1000;
	rep_next = (next * KEYBOARD_SCAN_FREQUENCY) / 1000;
}

/* getkey_repeat_filter(): Set the repeat filter function */
void getkey_repeat_filter(int (*filter)(int key, int duration, int count))
{
	filter_function = filter;
}
