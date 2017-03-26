#include <keyboard.h>

/*
	key_type()
	Returns a key's type. This functions ignores modifiers and expects
	matrix codes as argument, not key_id() codes.
*/
key_type_t key_type(int matrix_key)
{
	matrix_key &= MOD_CLEAR;

	// Arrow keys.
	if(matrix_key == KEY_UP || matrix_key == KEY_RIGHT
		|| matrix_key == KEY_DOWN || matrix_key == KEY_LEFT)
	{
		return key_type_arrow;
	}

	// Function keys (F1 .. F6) are ton only keys of the ninth row.
	if((matrix_key & 0x0f) == 0x09) return key_type_function;

	// Then character keys are those that have an associated character. =p
	return key_char(matrix_key) ? key_type_character : key_type_control;
}
