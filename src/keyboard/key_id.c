#include <keyboard.h>

/*
	keyid()
	Transforms a key identifier and returns a key code that is more
	convenient for array subscript that the original matrix codes. Please
	note that there are a few holes in the numbering.
	This function ignores modifiers and returns -1 on error.
*/
int key_id(int matrix_key)
{
	if(matrix_key < 0) return -1;
	matrix_key &= MOD_CLEAR;

	int row = 9 - (matrix_key & 0x0f);
	int column = 6 - ((matrix_key & 0xf0) >> 4);

	return 6 * row + column;
}
