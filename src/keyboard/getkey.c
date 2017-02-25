#include <internals/keyboard.h>
#include <keyboard.h>
#include <events.h>
#include <screen.h>

/*
	getkey()
	Blocking function with auto-repeat and SHIFT modifying functionalities.
	Roughly reproduces the behavior of the system's GetKey().
*/
int getkey(void)
{
	return getkey_opt(
		Getkey_ShiftModifier	|
		Getkey_AlphaModifier	|
		Getkey_ManageBacklight	|
		Getkey_RepeatArrowKeys,
		0
	);
}

/*
	getkey_opt()
	Enhances getkey() with more general functionalities.
	If max_cycles is non-zero and positive, getkey_opt() will return
	KEY_NOEVENT if no event occurs during max_cycle analysis.
*/
static void getkey_opt_wait(int *cycles)
{
	while(!interrupt_flag) sleep();
	interrupt_flag = 0;

	if(*cycles > 0) (*cycles)--;
}
int getkey_opt(enum GetkeyOpt options, int cycles)
{
	event_t event;
	int modifier = 0;
	static int event_ref = 0;

	if(cycles <= 0) cycles = -1;
	while(cycles != 0)
	{
		event = pollevent();
		switch(event.type)
		{
		case ET_None:
			if(last_key == KEY_NONE)
			{
				getkey_opt_wait(&cycles);	
				continue;
			}

			// Handling repetitions.
			enum KeyType type = keytype(last_key);
			if(!(options & (type << 4))) break;

			if(event_ref <= 0)
			{
				getkey_opt_wait(&cycles);
				event_ref++;
				continue;
			}

			last_events++;
			event_ref--;

			if(last_events >= (last_repeats ? repeat_next :
				repeat_first))
			{
				last_repeats++;
				last_events = 0;
				return last_key;
			}
			break;

		case ET_KeyPress:
			;
			int key = event.key;
			if(options & Getkey_ManageBacklight && key == KEY_OPTN
				&& modifier & MOD_SHIFT)
			{
				screen_toggleBacklight();
				modifier &= ~MOD_SHIFT;
				continue;
			}
			if(options & Getkey_ShiftModifier && key == KEY_SHIFT)
			{
				modifier ^= MOD_SHIFT;
				continue;
			}
			if(options & Getkey_AlphaModifier && key == KEY_ALPHA)
			{
				modifier ^= MOD_ALPHA;
				continue;
			}

			last_key	= key;
			last_repeats	= 0;
			last_events	= 0;
			event_ref	= 0;
			return key | modifier;

		case ET_KeyRel:
			if(event.key != last_key) break;
			last_key	= KEY_NONE;
			last_repeats	= 0;
			last_events	= 0;
			event_ref	= 0;
			break;

		default:
			break;
		}
	}

	last_key	= KEY_NONE;
	last_repeats	= 0;
	last_events	= 0;
	event_ref	= 0;
	return KEY_NONE;
}
