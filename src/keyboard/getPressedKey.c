#include <internals/keyboard.h>

/*
	getPressedKey()
	Finds a pressed key in the keyboard state and returns it.
*/
int getPressedKey(volatile uint8_t *keyboard_state)
{
	int row = 1, column = 0;
	int state;
	if(keyboard_state[0] & 1) return KEY_AC_ON;

	while(row <= 9 && !keyboard_state[row]) row++;
	if(row > 9) return KEY_NONE;

	state = keyboard_state[row];
	while(!(state & 1))
	{
		state >>= 1;
		column++;
	}

	return (column << 4) | row;
}
