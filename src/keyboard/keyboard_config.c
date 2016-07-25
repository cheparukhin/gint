#include <internals/keyboard.h>
#include <keyboard.h>

/*
	keyboard_setFrequency()
	Sets the keyboard frequency (in Hz).
*/
void keyboard_setFrequency(int frequency)
{
}

/*
	keyboard_setRepeatRate()
	Sets the default repeat rate for key events. The unit for the argument
	is the keyboard period.
	For example at 32 Hz, values of (20, 4) will imitate the system
	default.
*/
void keyboard_setRepeatRate(int first, int next)
{
	if(first < 0) first = 0;
	if(next < 0) next = 0;

	repeat_first = first;
	repeat_next = next;
}
