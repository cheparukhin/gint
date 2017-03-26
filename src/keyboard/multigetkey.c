#include <keyboard.h>
#include <internals/keyboard.h>
#include <events.h>

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
static void multigetkey_wait(int *cycles)
{
	while(!interrupt_flag) sleep();
	interrupt_flag = 0;

	if(*cycles > 0) (*cycles)--;
}
void multigetkey(int *keys, int count, int cycles)
{
	event_t event;
	int number = 0;

	if(count <= 0) return;
	if(cycles <= 0) cycles = -1;

	while(cycles != 0)
	{
		number = getPressedKeys(keyboard_state, keys, count);

		// We want to update the last key data when multigetkey()
		// returns a single key, because getkey() could be called a
		// short time after we return, and send a new event for this
		// key.
		if(number == 1)
		{
			last_key = keys[0];
			last_repeats = 0;
			last_time = 0;
		}

		if(number) break;
		multigetkey_wait(&cycles);
	}

	do event = pollevent();
	while(event.type != event_none);

	return;
}
