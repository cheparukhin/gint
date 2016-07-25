#include <internals/keyboard.h>
#include <keyboard.h>

/*
	getkey()
	Blocking function with auto-repeat and SHIFT modifying functionalities.
	Roughly reproduces the behavior of the system's GetKey().
*/
int getkey(void)
{
	return getkey_opt(
		Getkey_ShiftModifier |
		Getkey_AlphaModifier |

		Getkey_RepeatArrowKeys,

		0
	);
}



/*
	getkey_opt()
	Enhances getkey() with most general functionalities.
	If max_cycles is non-zero and positive, getkey_opt() will return
	KEY_NOEVENT if no event occurs during max_cycle analysis.
*/
int getkey_opt(enum GetkeyOpt options, int max_cycles)
{
	int key;
	enum KeyType type;
	int modifier = 0, last_modifier = KEY_NONE;
	int r;

	if(!max_cycles) max_cycles = -1;

	while(max_cycles != 0)
	{
		while(!interrupt_flag) sleep();
		interrupt_flag = 0;
		if(max_cycles > 0) max_cycles--;

		// Getting key and adding modifiers.
		key = getPressedKey(keyboard_state);

		// Handling "no_key" event;
		if(key == KEY_NONE)
		{
			// Condition for returning.
			r = (last_key != KEY_NONE &&
				options & Getkey_ReleaseEvent);

			last_key = KEY_NONE;
			last_modifier = KEY_NONE;
			last_repeats = 0;
			last_events = 0;

			if(r) return KEY_NONE;
		}

		// Handling "new key" events.
		else if(key != last_key)
		{
			if(options & Getkey_ShiftModifier && key == KEY_SHIFT)
			{
				if(last_modifier != KEY_SHIFT)
					modifier ^= MOD_SHIFT;
				last_modifier = KEY_SHIFT;

				continue;
			}
			if(options & Getkey_AlphaModifier && key == KEY_ALPHA)
			{
				if(last_modifier != KEY_ALPHA)
					modifier ^= MOD_ALPHA;
				last_modifier = KEY_ALPHA;

				continue;
			}

			last_key = key;
			last_repeats = 0;
			last_events = 0;

			return key | modifier;
		}

		// Handling key repetitions.
		else
		{
			type = keytype(key);

			// Checking whether this key type is repeated.
			if(options & (type << 4))
			{
				last_events++;
				r = last_repeats ? repeat_next : repeat_first;

				if(last_events >= r)
				{
					last_repeats++;
					last_events = 0;

					return key;
				}
			}
		}
	}

	// When no key was pressed during the given delay...
	return KEY_NOEVENT;
}
