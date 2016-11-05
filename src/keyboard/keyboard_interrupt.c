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

static void push_press(int keycode)
{
	struct Event event = {
		.type	= ET_KeyPress,
		.key	= keycode,
	};
	event_push(event);
}

static void push_release(int keycode)
{
	struct Event event = {
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
	unsigned char state[10] = { 0 };

	isSH3()	? keyboard_updateState_7705(state)
		: keyboard_updateState_7305(state)
	;

	// Try to minimize number of operations in common cases... this handles
	// AC/ON.
	if(keyboard_state[0] ^ state[0])
	{
		unsigned char pressed	= ~keyboard_state[0] &  state[0];
		unsigned char released	=  keyboard_state[0] & ~state[0];

		if(pressed & 1)  push_press(KEY_AC_ON);
		if(released & 1) push_release(KEY_AC_ON);
	}
	keyboard_state[0] = state[0];

	for(int row = 1; row <= 9; row++)
	{
		unsigned char pressed	= ~keyboard_state[row] &  state[row];
		unsigned char released	=  keyboard_state[row] & ~state[row];
		keyboard_state[row] = state[row];

		// Fasten this a bit.
		if(!pressed && !released) continue;

		for(int column = 0; column < 8; column++)
		{
			if(pressed & 1)  push_press  ((column << 4) | row);
			if(released & 1) push_release((column << 4) | row);

			pressed >>= 1;
			released >>= 1;
		}
	}

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
