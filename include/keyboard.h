#ifndef	_KEYBOARD_H
#define	_KEYBOARD_H	1

//---
//	Keycodes and related.
//---

// The following codes are gint matrix codes. They are not compatible with the
// system's.

#define	KEY_F1		0x69
#define	KEY_F2		0x59
#define	KEY_F3		0x49
#define	KEY_F4		0x39
#define	KEY_F4		0x39
#define	KEY_F5		0x29
#define	KEY_F6		0x19

#define	KEY_SHIFT	0x68
#define	KEY_OPTN	0x58
#define	KEY_VARS	0x48
#define	KEY_MENU	0x38
#define	KEY_LEFT	0x28
#define	KEY_UP		0x18

#define	KEY_ALPHA	0x67
#define	KEY_SQUARE	0x57
#define	KEY_POWER	0x47
#define	KEY_EXIT	0x37
#define	KEY_DOWN	0x27
#define	KEY_RIGHT	0x17

#define	KEY_XOT		0x66
#define	KEY_LOG		0x56
#define	KEY_LN		0x46
#define	KEY_SIN		0x36
#define	KEY_COS		0x26
#define	KEY_TAN		0x16

#define	KEY_FRAC	0x65
#define	KEY_FD		0x55
#define	KEY_LEFTP	0x45
#define	KEY_RIGHTP	0x35
#define	KEY_COMMA	0x25
#define	KEY_ARROW	0x15

#define	KEY_7		0x64
#define	KEY_8		0x54
#define	KEY_9		0x44
#define	KEY_DEL		0x34
#define	KEY_AC_ON	0x24

#define	KEY_4		0x63
#define	KEY_5		0x53
#define	KEY_6		0x43
#define	KEY_MUL		0x33
#define	KEY_DIV		0x23

#define	KEY_1		0x62
#define	KEY_2		0x52
#define	KEY_3		0x42
#define	KEY_PLUS	0x32
#define	KEY_MINUS	0x22

#define	KEY_0		0x61
#define	KEY_DOT		0x51
#define	KEY_EXP		0x41
#define	KEY_NEG		0x31
#define	KEY_EXE		0x21

// Key modifiers.
#define	MOD_SHIFT	0x80
#define	MOD_ALPHA	0x100
#define	MOD_CLEAR	~(MOD_SHIFT | MOD_ALPHA)

// Key events.
#define	KEY_NONE	0x00
#define	KEY_NOEVENT	0xff



//---
//	Keyboard configuration.
//---

/*
	enum KeyboardFrequency
	Describes the various frequencies available for the keyboard analysis.
	Default frequency is 16 Hz (system frequency is about 40 Hz). Very few
	applications will need to change this setting.
	Be aware that you will miss key hits at low frequencies.
	At high frequencies, you will lose important execution power. Some
	loop-driven programs will freeze at 64 Hz because they will never
	leave the interrupt handling routine. SH3 also have difficulties with
	64 Hz.
*/
enum KeyboardFrequency {
	KeyFreq_500mHz	= 7,
	KeyFreq_1Hz	= 6,
	KeyFreq_2Hz	= 5,
	KeyFreq_4Hz	= 4,
	KeyFreq_16Hz	= 3,
	KeyFreq_64Hz	= 2,
	KeyFreq_256Hz	= 1,
};

/*
	keyboard_setFrequency()
	Sets the keyboard frequency. Does nothing when the argument is not a
	valid KeyboardFrequency value.

	@arg	frequency
*/
void keyboard_setFrequency(enum KeyboardFrequency frequency);

/*
	keyboard_setRepeatRate()
	Sets the default repeat rate for key events. The unit for the argument
	is the keyboard period.
	For example at 16 Hz, values of (10, 2) will imitate the system
	default.
	Set to 0 to disable repetition. If first = 0, no repetition will be
	allowed. If first != 0 and next = 0, only one repetition will be
	allowed.

	@arg	first	Delay before first repeat, in keyboard period units.
	@arg	next	Delay before following repeats, in keyboard period
			units.
*/
void keyboard_setRepeatRate(int first, int next);



//---
//	Keyboard access.
//---

/*
	enum GetKeyOpt
	Options available for use with getkey_opt(). 
*/
enum GetkeyOpt
{
	Getkey_NoOption		= 0x00,

	// Return KEY_NONE when a key is released.
	Getkey_ReleaseEvent	= 0x01,

	// Consider [SHIFT] and [ALPHA] as modifiers instead of returning
	// KEY_SHIFT and KEY_ALPHA.
	Getkey_ShiftModifier	= 0x02,
	Getkey_AlphaModifier	= 0x04,

	// Key repetition. Notice that modifiers will never be repeated.
	Getkey_RepeatArrowKeys	= 0x10,
	Getkey_RepeatCharKeys	= 0x20,
	Getkey_RepeatCtrlKeys	= 0x40,
	Getkey_RepeatFuncKeys	= 0x80,
	// Shorthand for the four previous properties.
	Getkey_RepeatAllKeys	= 0xf0,
};

/*
	keylast()
	Returns the matrix code of the last pressed key. If repeat_count is
	non-NULL, it is set to the number of repetitions.

	@arg	repeat_count
	@return	Key matrix code.
*/
int keylast(int *repeat_count);

/*
	getkey()
	Blocking function with auto-repeat and SHIFT modifying functionalities.
	Reproduces the behavior of the system's GetKey().

	@return	Pressed key matrix code.
*/
int getkey(void);

/*
	getkey_opt()
	Enhances getkey() with most general functionalities.
	If max_cycles is non-zero and positive, getkey_opt() will return
	KEY_NOEVENT if no event occurs during max_cycle analysis.

	@arg	options		OR-combination of GetkeyOpt values.
	@arg	max_cycles

	@return	Pressed key matrix code.
*/
int getkey_opt(enum GetkeyOpt options, int max_cycles);

/*
	multigetkey()
	Listens the keyboard for simultaneous key hits. Uses the same options
	as getkey_opt().
	multigetkey() fills the 'keys' array with 'count' key codes, adding
	KEY_NONE if less than 'count' keys are pressed.
	Be aware that rectangle and column effects can make multigetkey() read
	unpressed keys as pressed (see documentation for more information).
	Setting count = 3 is generally safe.

	@arg	keys		Key code array.
	@arg	count		Maximum number of keys that will be read.
	@arg	max_cycles
*/
void multigetkey(int *keys, int count, int max_cycles);



//---
//	Key analysis.
//---

enum KeyType
{
	KeyType_Arrow		= 1,
	KeyType_Character	= 2,
	KeyType_Control		= 4,
	KeyType_Function	= 8,
};

/*
	keyid()
	Returns a non-matrix key code that can be used for array subscript.
	Ignores modifiers.

	@arg	key
	@return	Modified keycode.
*/
int keyid(int key);

/*
	keychar()
	Returns the ASCII character associated with a character key ; 0 for
	other keys.

	@arg	key
	@return	Key character.
*/
int keychar(int key);

/*
	keytype()
	Returns a key's type. Ignores modifiers.

	@arg	key
	@return	Key type.
*/
enum KeyType keytype(int key);

#endif	// _KEYBOARD_H
