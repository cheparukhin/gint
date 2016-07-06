#include <keyboard_internals.h>

/*
	sleep()
	Puts the CPU to sleep and waits for an interrupt.
*/
void sleep(void)
{
	__asm__
	(
		"sleep\n\t"
	);
}
