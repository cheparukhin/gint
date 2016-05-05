#include <keyboard.h>
#include <timer.h>
#include <mpu.h>

//---
//	Keyboard variables.
//---

// These ones get modified by interrupts.
static volatile unsigned char keyboard_state[10] = { 0 };
static volatile int interrupt_flag = 0;

// Key statistics.
static int repeat_first = 10, repeat_next = 2;
static int last_key = KEY_NONE, last_repeats = 0, last_events = 0;



//---
//	Auxiliary functions.
//---

/*
	sleep()
	Puts the CPU to sleep and waits for an interrupt.
*/
static void sleep(void)
{
	__asm__
	(
		"sleep\n\t"
	);
}

/*
	getPressedKey()
	Finds a pressed key in the keyboard state and returns it.

	@return	A pressed key.
*/
static int getPressedKey(void)
{
	int row = 1, column = 0;
	int state;
	if(keyboard_state[0] & 1) return KEY_AC_ON;

	while(row <= 9 && !keyboard_state[row]) row++;
	if(row > 9) return KEY_NONE;

	state = keyboard_state[row];
	while(!(state & 1))
	{
		state >>= 1;
		column++;
	}

	return (column << 4) | row;
}

/*
	getPressedKeys()
	Find 'count' pressed keys in the keyboard state.

	@arg	keys	Will be filled.
	@arg	count	Size of array.

	@return	Number of actual pressed keys found.
*/
static int getPressedKeys(int *keys, int count)
{
	int row = 1, column;
	int found = 0, actually_pressed;
	int state;
	if(count <= 0) return 0;

	if(keyboard_state[0] & 1)
	{
		keys[found++] = KEY_AC_ON;
		count--;
	}

	while(count && row <= 9)
	{
		while(row <= 9 && !keyboard_state[row]) row++;
		if(row > 9) break;

		state = keyboard_state[row];
		column = 0;

		while(count && column < 8)
		{
			if(state & 1)
			{
				keys[found++] = (column << 4) | row;
				count--;
			}

			state >>= 1;
			column++;
		}

		row++;
	}

	actually_pressed = found;

	while(count)
	{
		keys[found++] = KEY_NONE;
		count--;
	}

	return actually_pressed;
}



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



//---
//	Keyboard configuration.
//---

/*
	keyboard_setFrequency()
	Sets the keyboard frequency.

	@arg	frequency	In Hz.
*/
void keyboard_setFrequency(int frequency)
{
}

/*
	keyboard_setRepeatRate()
	Sets the default repeat rate for key events. The unit for the argument
	is the keyboard period.
	For example at 16 Hz, values of (10, 2) will imitate the system
	default.

	@arg	first	Delay before first repeat, in keyboard period units.
	@arg	next	Delay before following repeats, in keyboard period
			units.
*/
void keyboard_setRepeatRate(int first, int next)
{
	if(first < 0) first = 0;
	if(next < 0) next = 0;

	repeat_first = first;
	repeat_next = next;
}



//---
//	Keyboard access.
//---

/*
	keylast()
	Returns the matrix code of the last pressed key. If repeat_count is
	non-NULL, it is set to the number of repetitions.

	@arg	repeat_count
	@return	Key matrix code.
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

	@return	10-byte keyboard state buffer.
*/
volatile unsigned char *keystate(void)
{
	return keyboard_state;
}




/*
	getkey()
	Blocking function with auto-repeat and SHIFT modifying functionalities.
	Roughly reproduces the behavior of the system's GetKey().

	@return	Pressed key matrix code.
*/
int getkey(void)
{
	return getkey_opt(
		Getkey_ShiftModifier |
		Getkey_AlphaModifier |

		Getkey_RepeatArrowKeys,

		0
	);
}



/*
	getkey_opt()
	Enhances getkey() with most general functionalities.
	If max_cycles is non-zero and positive, getkey_opt() will return
	KEY_NOEVENT if no event occurs during max_cycle analysis.

	@arg	options		OR-combination of GetkeyOpt values.
	@arg	max_cycles

	@return	Pressed key matrix code.
*/
int getkey_opt(enum GetkeyOpt options, int max_cycles)
{
	int key;
	enum KeyType type;
	int modifier = 0, last_modifier = KEY_NONE;
	int r;

	if(!max_cycles) max_cycles = -1;

	while(max_cycles != 0)
	{
		while(!interrupt_flag) sleep();
		interrupt_flag = 0;
		if(max_cycles > 0) max_cycles--;

		// Getting key and adding modifiers.
		key = getPressedKey();

		// Handling "no_key" event;
		if(key == KEY_NONE)
		{
			// Condition for returning.
			r = (last_key != KEY_NONE &&
				options & Getkey_ReleaseEvent);

			last_key = KEY_NONE;
			last_modifier = KEY_NONE;
			last_repeats = 0;
			last_events = 0;

			if(r) return KEY_NONE;
		}

		// Handling "new key" events.
		else if(key != last_key)
		{
			if(options & Getkey_ShiftModifier && key == KEY_SHIFT)
			{
				if(last_modifier != KEY_SHIFT)
					modifier ^= MOD_SHIFT;
				last_modifier = KEY_SHIFT;

				continue;
			}
			if(options & Getkey_AlphaModifier && key == KEY_ALPHA)
			{
				if(last_modifier != KEY_ALPHA)
					modifier ^= MOD_ALPHA;
				last_modifier = KEY_ALPHA;

				continue;
			}

			last_key = key;
			last_repeats = 0;
			last_events = 0;

			return key | modifier;
		}

		// Handling key repetitions.
		else
		{
			type = keytype(key);

			// Checking whether this key type is repeated.
			if(options & (type << 4))
			{
				last_events++;
				r = last_repeats ? repeat_next : repeat_first;

				if(last_events >= r)
				{
					last_repeats++;
					last_events = 0;

					return key;
				}
			}
		}
	}

	// When no key was pressed during the given delay...
	return KEY_NOEVENT;
}

/*
	multigetkey()
	Listens the keyboard for simultaneous key hits. Uses the same options
	as getkey_opt().
	multigetkey() fills the 'keys' array with 'count' key codes, adding
	KEY_NOKEY if less than 'count' keys are pressed.
	Be aware that rectangle and column effects can make multigetkey() read
	unpressed keys as pressed (see documentation for more information).
	Setting count = 3 is generally safe.

	@arg	keys		Key code array.
	@arg	count		Maximum number of keys that will be read.
	@arg	max_cycles
*/
void multigetkey(int *keys, int count, int max_cycles)
{
	int number;

	if(!max_cycles) max_cycles = -1;

	while(max_cycles != 0)
	{
		while(!interrupt_flag) sleep();
		interrupt_flag = 0;
		if(max_cycles > 0) max_cycles--;

		number = getPressedKeys(keys, count);

		// We need to update the last key data, in case multigetkey()
		// returns a single key, and getkey() is called a short time
		// after. Otherwise getkey() could re-send an event for this
		// key.
		if(number == 1)
		{
			last_key = keys[0];
			last_repeats = 0;
			last_events = 0;
		}

		if(number) return;

		// Handle key repetitions.
/*
		else
		{
			type = keytype(key);

			// Checking whether this key type is repeated.
			if(options & (type << 4))
			{
				last_events++;
				r = last_repeats ? repeat_next : repeat_first;

				if(last_events >= r)
				{
					last_repeats++;
					last_events = 0;

					return key;
				}
			}
		}
*/
	}

	// When no key was pressed during the given delay... (no need to fill
	// the array, it has already been done by getPressedKeys()).
	return;
}



//---
//	Key analysis.
//---

/*
	keyid()
	Returns a non-matrix key code that can be used for array subscript.
	Ignores modifiers.

	@arg	key
	@return	Modified keycode.
*/
int keyid(int key)
{
	if(key < 0) return -1;
	key &= MOD_CLEAR;

	int row = 9 - (key & 0x0f);
	int column = 6 - ((key & 0xf0) >> 4);

	return 6 * row + column;
}

/*
	keychar()
	Returns the ASCII character associated with a key, or 0 for control
	keys.

	@arg	key
*/
int keychar(int key)
{
	char flat[] = {
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		0x0, '2', '^', 0x0, 0x0, 0x0,
		'X', 'l', 'l', 's', 'c', 't',
		0x0, 0x0, '(', ')', ',', '>',
		'7', '8', '9', 0x0, 0x0, 0x0,
		'4', '5', '6', '*', '/', 0x0,
		'1', '2', '3', '+', '-', 0x0,
		'0', '.', 'e', '-', 0x0, 0x0
	};
	char alpha[] = {
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		0x0, 'r', 'o', 0x0, 0x0, 0x0,
		'A', 'B', 'C', 'D', 'E', 'F',
		'G', 'H', 'I', 'J', 'K', 'L',
		'M', 'N', 'O', 0x0, 0x0, 0x0,
		'P', 'Q', 'R', 'S', 'T', 0x0,
		'U', 'V', 'W', 'X', 'Y', 0x0,
		'Z', ' ', '"', 0x0, 0x0, 0x0
	};

	int id = keyid(key);

	if(key & MOD_ALPHA) return alpha[id];
	return flat[id];
}

/*
	keytype()
	Returns a key's type. Ignores modifiers.

	@arg	key
	@return	Key type.
*/
enum KeyType keytype(int key)
{
	key &= MOD_CLEAR;

	if(key == KEY_UP || key == KEY_RIGHT || key == KEY_DOWN ||
		key == KEY_LEFT) return KeyType_Arrow;

	if((key & 0x0f) == 0x09) return KeyType_Function;

	return keychar(key) ? KeyType_Character : KeyType_Control;
}
