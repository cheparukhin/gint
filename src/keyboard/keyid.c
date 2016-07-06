#include <keyboard.h>

/*
	keyid()
	Returns a non-matrix key code that can be used for array subscript.
	Ignores modifiers.
*/
int keyid(int key)
{
	if(key < 0) return -1;
	key &= MOD_CLEAR;

	int row = 9 - (key & 0x0f);
	int column = 6 - ((key & 0xf0) >> 4);

	return 6 * row + column;
}
