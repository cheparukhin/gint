#include <internals/keyboard.h>
#include <internals/timer.h>
#include <keyboard.h>
#include <events.h>
#include <screen.h>
#include <gint.h>

/*
	getkey()
	Blocking function with auto-repeat and SHIFT modifying functionalities.
	Roughly reproduces the behavior of the system's GetKey().
*/
int getkey(void)
{
	return getkey_opt(
		getkey_shift_modifier		|
		getkey_alpha_modifier		|
		getkey_manage_backlight		|
		getkey_task_switch		|
		getkey_repeat_arrow_keys,

		0
	);
}

/*
	getkey_opt()
	Enhances getkey() with more general functionalities.
	If delay_ms is non-zero and positive, getkey_opt() will return
	KEY_NOEVENT if no event occurs before delay_ms.
*/
static void getkey_opt_wait(int *delay_ms)
{
	while(!interrupt_flag) sleep();
	interrupt_flag = 0;

	if(*delay_ms > 0)
	{
		(*delay_ms) -= vtimer->ms_delay;
		if(*delay_ms < 0) *delay_ms = 0;
	}
}
int getkey_opt(getkey_option_t options, int delay_ms)
{
	event_t event;
	int modifier = 0, key;
	key_type_t type;

	if(delay_ms <= 0) delay_ms = -1;

	while(delay_ms != 0) switch((event = pollevent()).type)
	{
	case event_none:
		getkey_opt_wait(&delay_ms);
		break;

	case event_key_press:
		key = event.key.code;

		if(options & getkey_manage_backlight && key == KEY_OPTN
			&& (modifier & MOD_SHIFT))
		{
			screen_toggleBacklight();
			modifier &= ~MOD_SHIFT;
			continue;
		}
		if((options & getkey_task_switch) && key == KEY_MENU
			&& !modifier)
		{
			continue;
		}
		if(options & getkey_shift_modifier && key == KEY_SHIFT)
		{
			modifier ^= MOD_SHIFT;
			continue;
		}
		if(options & getkey_alpha_modifier && key == KEY_ALPHA)
		{
			modifier ^= MOD_ALPHA;
			continue;
		}

		last_key	= key;
		last_repeats	= 0;
		last_time	= 0;
		return key | modifier;

	case event_key_repeat:
		key = event.key.code;
		if(key != last_key) continue;

		// Checking that this type of repetition is allowed.
		type = key_type(key);
		if(!(options & (type << 4))) break;

		last_time += vtimer->ms_delay;
		int cmp = last_repeats ? repeat_next : repeat_first;

		if(last_time >= cmp)
		{
			last_repeats++;
			last_time -= cmp;
			return last_key;
		}
		break;

	case event_key_release:
		if((options & getkey_task_switch) && event.key.code == KEY_MENU
			&& !modifier)
		{
			gint_switch();
			continue;
		}

		if((int)event.key.code != last_key) break;
		last_key	= KEY_NONE;
		last_repeats	= 0;
		last_time	= 0;
		break;

	default:
		break;
	}

	last_key	= KEY_NONE;
	last_repeats	= 0;
	last_time	= 0;
	return KEY_NONE;
}
