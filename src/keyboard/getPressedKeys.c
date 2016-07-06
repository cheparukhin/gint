#include <keyboard_internals.h>

/*
	getPressedKeys()
	Find 'count' pressed keys in the keyboard state and fills the 'keys'
	array. Returns the number of actually-pressed keys found.
*/
int getPressedKeys(volatile unsigned char *keyboard_state, int *keys,
	int count)
{
	int row = 1, column;
	int found = 0, actually_pressed;
	int state;
	if(count <= 0) return 0;

	if(keyboard_state[0] & 1)
	{
		keys[found++] = KEY_AC_ON;
		count--;
	}

	while(count && row <= 9)
	{
		while(row <= 9 && !keyboard_state[row]) row++;
		if(row > 9) break;

		state = keyboard_state[row];
		column = 0;

		while(count && column < 8)
		{
			if(state & 1)
			{
				keys[found++] = (column << 4) | row;
				count--;
			}

			state >>= 1;
			column++;
		}

		row++;
	}

	actually_pressed = found;

	while(count)
	{
		keys[found++] = KEY_NONE;
		count--;
	}

	return actually_pressed;
}
