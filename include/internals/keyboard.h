#ifndef	_INTERNALS_KEYBOARD_H
#define	_INTERNALS_KEYBOARD_H	1

#include <keyboard.h>

// Keyboard variables.
extern volatile unsigned char keyboard_state[10];
extern volatile int interrupt_flag;

// Key statistics.
extern int repeat_first, repeat_next;
extern int last_key, last_repeats, last_events;

// RTC callback id.
extern unsigned cb_id;

/*
	sleep()
	Puts the CPU into sleep until an interrupt request is accepted.
*/
void sleep(void);

/*
	getPressedKey()
	Finds a pressed key in the keyboard state and returns it.
*/
int getPressedKey(volatile unsigned char *keyboard_state);

/*
	getPressedKeys()
	Find 'count' pressed keys in the keyboard state and fills the 'keys'
	array. Returns the number of keys found.
	WARNING: keyboard artifacts make this function read as pressed keys
	that aren't (typically, LEFT + DOWN + SHIFT => ALPHA).
*/
int getPressedKeys(volatile unsigned char *keyboard_state, int *keys,
	int count);

#endif	// _INTERNALS_KEYBOARD_H
