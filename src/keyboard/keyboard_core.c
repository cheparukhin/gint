#include <internals/keyboard.h>
#include <keyboard.h>
#include <timer.h>
#include <mpu.h>
#include <clock.h>
#include <events.h>

//---
//	Keyboard variables.
//---

// Current keyboard state: each element represents a row, and each bit a key,
// but not all bits are used.
volatile uint8_t keyboard_state[10] = { 0 };
// Interrupt flag: set when an interrupt occurs, and cleared by functions such
// as getkey() that watch it (because such functions will wake up every time an
// interrupt occurs, and they need to know whether it was the keyboard).
volatile int interrupt_flag = 0;

// Delays, in milliseconds, before repeating keys (the first repetition may
// have a different delay).
int repeat_first = 625, repeat_next = 125;

// Which key was pressed last, how many times it has been repeated, and how
// much time (in milliseconds) has elapsed since it was last repeated.
int last_key = KEY_NONE, last_repeats = 0, last_time = 0;

// Virtual timer object.
timer_t *vtimer = NULL;



//---
//	Interrupt management.
//---

static inline void push_press(int keycode)
{
	uint32_t id = key_id(keycode);

	event_t event = {
		.type		= event_key_press,
		.key.code	= keycode,
		.key.id		= id,
		.key.character	= key_char(keycode),
	};
	event_push(event);
}

static inline void push_repeat(int keycode)
{
	uint32_t id = key_id(keycode);

	event_t event = {
		.type		= event_key_repeat,
		.key.code	= keycode,
		.key.id		= id,
		.key.character	= key_char(keycode),
	};
	event_push(event);
}

static inline void push_release(int keycode)
{
	uint32_t id = key_id(keycode);

	event_t event = {
		.type		= event_key_release,
		.key.code	= keycode,
		.key.id		= id,
		.key.character	= key_char(keycode),
	};
	event_push(event);
}

/*
	keyboard_interrupt()
	Callback function for keyboard update; called by the timer manager when
	the keyboard's virtual timer fires. Ideally this function should be
	interrupt-driven but the PINT interrupts seem to fire continuously,
	which is annoying.
*/
void keyboard_interrupt(void)
{
	uint8_t state[10] = { 0 };

	isSH3()	? keyboard_updateState_7705(state)
		: keyboard_updateState_7305(state);

	// This procedure really needs to be speed-optimized... and it's hard
	// because of this bit manipulation. This condition handles AC/ON.
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

		// Make this a bit faster.
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

	// Signal the interrupt to the higher-level functions.
	interrupt_flag = 1;
}



//---
//	Keyboard configuration.
//---

/*
	keyboard_init()
	Starts the keyboard timer.
*/
__attribute__((constructor)) void keyboard_init(void)
{
	keyboard_setRepeatRate(625, 125);

	vtimer = timer_create(25, 0);
	timer_attach(vtimer, keyboard_interrupt, NULL);
	timer_start(vtimer);
}

/*
	keyboard_setAnalysisDelay()
	Sets the keyboard analysis delay, that is, the delay (in ms) between
	two keyboard analyzes. Please note that the repeat delays should be
	multiples of the analysis delay for better accuracy.
*/
void keyboard_setAnalysisDelay(int analysis_delay_ms)
{
	if(analysis_delay_ms <= 0) return;
	timer_reload(vtimer, analysis_delay_ms);
}

/*
	keyboard_setRepeatRate()
	Sets the default repeat rate for key events. The delay before the first
	repeat may have a different value (usually longer). The unit for the
	argument is ms, but the repeat events themselves may only be fired when
	a keyboard analysis is performed; which means that for better accuracy,
	these delays should be a multiple of the keyboard period.
	For instance, delays of (625 ms, 125 ms) will imitate the system's
	default setting.
*/
void keyboard_setRepeatRate(int first, int next)
{
	repeat_first	= (first > 0) ? first : 0;
	repeat_next	= (next  > 0) ? next  : 0;
}

/*
	keyboard_quit()
	Stops the keyboard timer.
*/
__attribute__((destructor)) void keyboard_quit(void)
{
	timer_destroy(vtimer);
	vtimer = NULL;
}

/*
	keyboard_stateBuffer()
	Returns the address of the keyboard state array. The returned address
	is the handler's buffer, therefore it contains volatile data.
*/
volatile uint8_t *keyboard_stateBuffer(void)
{
	return keyboard_state;
}
