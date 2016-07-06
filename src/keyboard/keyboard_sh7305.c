//---
//
//	gint core module: keyboard analyzer
//
//	Probably the most difficult hardware interaction. There is very few
//	documentation on how the system actually analyzes the keyboard. While
//	disassembling syscalls reveals the following procedure (which was
//	already documented by SimonLothar), there is nothing about the
//	detection problems of the multi-getkey system.
//
//---

#include <keyboard.h>
#include <7305.h>

//---
//	Keyboard management.
//---

/*
	kdelay()
	Should sleep during a few milliseconds. Well...
	This delay has a great influence on key detection. Here are a few
	observations of what happens with common numbers of "nop" (without
	overclock). Take care of the effect of overclock !

	Column effects
	May have something to do with register HIZCRB not being used here. When
	many keys on the same column are pressed, other keys of the same column
	may be triggered.

	- Less	Bad key detection.
	- 8	Very few column effects. Most often, three keys may be pressed
		simultaneously. However, [UP] has latencies and is globally not
		detected.
	- 12	Seems best. Every individual key is detected well. No column
		effect observed before four keys.
	- 16	Every single key is detected correctly. Pressing two keys on a
		same column does not usually create a column effect. Three keys
		almost always.
	- More	Does not improve single key detection, and increase column
		effects. At 256 every single key press create a whole column
		effect.
*/
static void kdelay(void)
{
	#define	r4(str)	str str str str

	__asm__
	(
		r4("nop\n\t")
		r4("nop\n\t")
		r4("nop\n\t")
	);

	#undef	r4
}

/*
	krow()
	Reads a keyboard row. Works like krow() for SH7705. See gint_7705.c for
	more details.
*/
static int krow(int row)
{
	volatile unsigned short *injector1 = (unsigned short *)0xa4050116;
	volatile unsigned char *data1 = (unsigned char *)0xa4050136;

	volatile unsigned short *injector2 = (unsigned short *)0xa4050118;
	volatile unsigned char *data2 = (unsigned char *)0xa4050138;

	volatile unsigned short *detector = (unsigned short *)0xa405014c;
	volatile unsigned char *keys = (unsigned char *)0xa405016c;

	volatile unsigned char *key_register = (unsigned char *)0xa40501c6;
//	volatile unsigned short *hizcrb = (unsigned short *)0xa405015a;

	unsigned short smask;
	unsigned char cmask;
	int result		= 0;

	if(row < 0 || row > 9) return 0;

	// Additional configuration for SH7305.
	*detector	= 0xaaaa;
	*key_register	= 0xff;
	*injector1	= (*injector1 & 0xf000) | 0x0555;
	*injector2	= (*injector2 & 0xf000) | 0x0555;
	*data1		|= 0x3f;
	*data2		|= 0x3f;
	kdelay();

	if(row < 6)
	{
		smask = 0x0003 << (row * 2);
		cmask = ~(1 << row);

		*injector1 = ((*injector1 & 0xf000) | 0x0aaa) ^ smask;
		*injector2 =  (*injector2 & 0xf000) | 0x0aaa;
		kdelay();

		*data1 = (*data1 & 0xc0) | cmask;
		*data2 |= 0x3f;
		kdelay();
	}
	else
	{
		smask = 0x0003 << ((row - 6) * 2);
		cmask = ~(1 << (row - 6));

		*injector1 =  (*injector1 & 0xf000) | 0x0aaa;
		*injector2 = ((*injector2 & 0xf000) | 0x0aaa) ^ smask;
		kdelay();

		*data1 |= 0x3f;
		*data2 = (*data2 & 0xc0) | cmask;
		kdelay();
	}

	// Reading the keyboard row.
	result = ~*keys;
	kdelay();

	// Re-initializing the port configuration and data.
	*injector1 = (*injector1 & 0xf000) | 0x0aaa;
	*injector2 = (*injector2 & 0xf000) | 0x0aaa;
	kdelay();
	*injector1 = (*injector1 & 0xf000) | 0x0555;
	*injector2 = (*injector2 & 0xf000) | 0x0555;
	kdelay();
	*data1 &= 0xc0;
	*data2 &= 0xc0;

	return result;
}

/*
	keyboard_updateState()
	Updates the keyboard state.
*/
void keyboard_updateState_7305(volatile unsigned char *keyboard_state)
{
	int i;

	for(i = 0; i < 10; i++) keyboard_state[i] = krow(i);
}
