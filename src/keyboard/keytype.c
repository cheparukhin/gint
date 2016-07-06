#include <keyboard.h>

/*
	keytype()
	Returns a key's type. Ignores modifiers.
*/
enum KeyType keytype(int key)
{
	key &= MOD_CLEAR;

	if(key == KEY_UP || key == KEY_RIGHT || key == KEY_DOWN ||
		key == KEY_LEFT) return KeyType_Arrow;

	if((key & 0x0f) == 0x09) return KeyType_Function;

	return keychar(key) ? KeyType_Character : KeyType_Control;
}
