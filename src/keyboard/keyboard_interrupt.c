#include <keyboard.h>
#include <timer.h>
#include <mpu.h>

#include <keyboard_internals.h>

//---
//	Keyboard variables.
//---

// These ones get modified by interrupts.
volatile unsigned char keyboard_state[10] = { 0 };
volatile int interrupt_flag = 0;

// Key statistics.
int repeat_first = 10, repeat_next = 2;
int last_key = KEY_NONE, last_repeats = 0, last_events = 0;


//---
//	Interrupt management.
//---

/*
	keyboard_interrupt()
	Callback for keyboard update. Allows keyboard analysis functions to
	wake only when keyboard interrupts happen.
*/
void keyboard_interrupt(void)
{
	if(isSH3())
		keyboard_updateState_7705(keyboard_state);
	else
		keyboard_updateState_7305(keyboard_state);

	interrupt_flag = 1;
}

/*
	keyboard_init()
	Starts the keyboard timer.
*/
void keyboard_init(void)
{
	timer_start(TIMER_KEYBOARD, 1700, TIMER_Po_256, keyboard_interrupt,
		0);
}

/*
	keyboard_quit()
	Stops the keyboard timer.
*/
void keyboard_quit(void)
{
	timer_stop(TIMER_KEYBOARD);
}
