#include <internals/keyboard.h>
#include <keyboard.h>
#include <timer.h>
#include <mpu.h>
#include <clock.h>
#include <events.h>

//---
//	Keyboard variables.
//---

// These ones get modified by interrupts.
volatile uint8_t keyboard_state[10] = { 0 };
volatile int interrupt_flag = 0;

// Key statistics.
int repeat_first = 10, repeat_next = 2;
int last_key = KEY_NONE, last_repeats = 0, last_events = 0;

// RTC callback id.
unsigned cb_id;



//---
//	Interrupt management.
//---

static void push_press(int keycode)
{
	event_t event = {
		.type	= ET_KeyPress,
		.key	= keycode,
	};
	event_push(event);
}

static void push_repeat(int keycode)
{
	event_t event = {
		.type	= ET_KeyRepeat,
		.key	= keycode,
	};
	event_push(event);
}

static void push_release(int keycode)
{
	event_t event = {
		.type	= ET_KeyRel,
		.key	= keycode,
	};
	event_push(event);
}

/*
	keyboard_interrupt()
	Callback for keyboard update. Allows keyboard analysis functions to
	wake only when keyboard interrupts happen.
*/
void keyboard_interrupt(void)
{
	uint8_t state[10] = { 0 };

	isSH3()	? keyboard_updateState_7705(state)
		: keyboard_updateState_7305(state)
	;

	// Try to minimize number of operations in common cases... this handles
	// AC/ON.
	if(keyboard_state[0] ^ state[0])
	{
		uint8_t pressed		= ~keyboard_state[0] &  state[0];
		uint8_t released	=  keyboard_state[0] & ~state[0];

		if(pressed & 1)  push_press(KEY_AC_ON);
		if(released & 1) push_release(KEY_AC_ON);
	}
	keyboard_state[0] = state[0];

	for(int row = 1; row <= 9; row++)
	{
		uint8_t pressed		= ~keyboard_state[row] &  state[row];
		uint8_t repeated	=  keyboard_state[row] &  state[row];
		uint8_t released	=  keyboard_state[row] & ~state[row];
		keyboard_state[row] = state[row];

		// Fasten this a bit.
		if(!(pressed | repeated | released)) continue;

		for(int column = 0; column < 8; column++)
		{
			if(pressed  & 1) push_press  ((column << 4) | row);
			if(repeated & 1) push_repeat ((column << 4) | row);
			if(released & 1) push_release((column << 4) | row);

			pressed  >>= 1;
			repeated >>= 1;
			released >>= 1;
		}
	}

	interrupt_flag = 1;
}

/*
	keyboard_init()
	Starts the keyboard timer.
*/
__attribute__((constructor)) void keyboard_init(void)
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
__attribute__((destructor)) void keyboard_quit(void)
{
	rtc_cb_end(cb_id);
}
