#include <keyboard.h>
#include <internals/keyboard.h>

/*
	multigetkey()
	Listens the keyboard for simultaneous key hits. Uses the same options
	as getkey_opt().
	multigetkey() fills the 'keys' array with 'count' key codes, adding
	KEY_NOKEY if less than 'count' keys are pressed.
	Be aware that rectangle and column effects can make multigetkey() read
	unpressed keys as pressed (see documentation for more information).
	Setting count = 3 is generally safe.
	The function returns after 'max_cycles' if no key is pressed.
*/
void multigetkey(int *keys, int count, int max_cycles)
{
	int number;

	if(!max_cycles) max_cycles = -1;

	while(max_cycles != 0)
	{
		while(!interrupt_flag) sleep();
		interrupt_flag = 0;
		if(max_cycles > 0) max_cycles--;

		number = getPressedKeys(keyboard_state, keys, count);

		// We need to update the last key data, in case multigetkey()
		// returns a single key, and getkey() is called a short time
		// after. Otherwise getkey() could re-send an event for this
		// key.
		if(number == 1)
		{
			last_key = keys[0];
			last_repeats = 0;
			last_events = 0;
		}

		if(number) return;
	}

	// When no key was pressed during the given delay... (no need to fill
	// the array, it has already been done by getPressedKeys()).
	return;
}
