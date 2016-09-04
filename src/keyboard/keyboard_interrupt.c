#include <keyboard.h>
#include <timer.h>
#include <mpu.h>
#include <clock.h>

#include <internals/keyboard.h>

//---
//	Keyboard variables.
//---

// These ones get modified by interrupts.
volatile unsigned char keyboard_state[10] = { 0 };
volatile int interrupt_flag = 0;

// Key statistics.
int repeat_first = 10, repeat_next = 2;
int last_key = KEY_NONE, last_repeats = 0, last_events = 0;

// RTC callback id.
unsigned cb_id;



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
	cb_id = rtc_cb_add(RTCFreq_16Hz, keyboard_interrupt, 0);
}

/*
	keyboard_setFrequency()
	Sets the keyboard frequency.
*/
void keyboard_setFrequency(enum KeyboardFrequency frequency)
{
	if(frequency < 1 || frequency > 7) return;
	rtc_cb_edit(cb_id, frequency, keyboard_interrupt);
}

/*
	keyboard_quit()
	Stops the keyboard timer.
*/
void keyboard_quit(void)
{
	rtc_cb_end(cb_id);
}
