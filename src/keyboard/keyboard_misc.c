#include <keyboard.h>
#include <internals/keyboard.h>

/*
	keylast()
	Returns the matrix code of the last pressed key. If repeat_count is
	non-NULL, it is set to the number of repetitions.
*/
int keylast(int *repeat_count)
{
	if(repeat_count) *repeat_count = last_repeats;
	return last_key;
}



/*
	keystate()
	Returns the address of the keyboard state array. The returned address
	is the handler's buffer, therefore it contains volatile data.
*/
volatile uint8_t *keystate(void)
{
	return keyboard_state;
}
