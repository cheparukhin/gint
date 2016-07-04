#include <gint.h>
#include <timer.h>
#include <keyboard.h>
#include <7305.h>

//---
//	Interrupt codes.
//---

#define	IC_RTC_PRI	0xaa0
#define	IC_KEYSC	0xbe0
#define	IC_TMU0_TUNI0	0x400
#define	IC_TMU0_TUNI1	0x420
#define	IC_TMU0_TUNI2	0x440



//---
//	Various MPU-dependent procedures.
//---

/*
	gint_setRTCFrequency()
	Sets the RTC interrupt frequency and enables interrupts.

	@arg	frequency
*/
void gint_setRTCFrequency_7305(enum RTCFrequency frequency)
{
	if(frequency < 1 || frequency > 7) return;
	RTC.RCR2.BYTE = (frequency << 4) | 0x09;
}

/*
	gint_getRTCFrequency()
	Returns the RTC interrupt frequency.

	@return	RTC interrupt frequency.
*/
enum RTCFrequency gint_getRTCFrequency_7305(void)
{
	return (RTC.RCR2.BYTE & 0x70) >> 4;
}



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



//---
//	Interrupt handler.
//---

void gint_7305(void)
{
	volatile unsigned int *intevt  = (unsigned int *)0xff000028;
	unsigned int code = *intevt;

	switch(code)
	{
	case IC_RTC_PRI:
		RTC.RCR2.PEF = 0;
		break;

	case IC_TMU0_TUNI0:
		timer_interrupt(TIMER_TMU0);
		break;

	case IC_TMU0_TUNI1:
		timer_interrupt(TIMER_TMU1);
		break;

	case IC_TMU0_TUNI2:
		timer_interrupt(TIMER_TMU2);
		break;
	}
}



//---
//	Setup.
//---

static unsigned short ipr[12];
static unsigned char rcr2;
// Saves of the keyboard registres. Could be better.
static unsigned short inj1, inj2, det;
static unsigned char data1, data2, keys, reg;

static void gint_priority_lock_7305(void)
{
	// Saving the current interrupt priorities.
	ipr[0]  = INTX.IPRA.WORD;
	ipr[1]  = INTX.IPRB.WORD;
	ipr[2]  = INTX.IPRC.WORD;
	ipr[3]  = INTX.IPRD.WORD;
	ipr[4]  = INTX.IPRE.WORD;
	ipr[5]  = INTX.IPRF.WORD;
	ipr[6]  = INTX.IPRG.WORD;
	ipr[7]  = INTX.IPRH.WORD;
	ipr[8]  = INTX.IPRI.WORD;
	ipr[9]  = INTX.IPRJ.WORD;
	ipr[10] = INTX.IPRK.WORD;
	ipr[11] = INTX.IPRL.WORD;

	// Disabling everything by default to avoid freezing on unhandled
	// interrupts.
	INTX.IPRA.WORD = 0x0000;
	INTX.IPRB.WORD = 0x0000;
	INTX.IPRC.WORD = 0x0000;
	INTX.IPRD.WORD = 0x0000;
	INTX.IPRE.WORD = 0x0000;
	INTX.IPRF.WORD = 0x0000;
	INTX.IPRG.WORD = 0x0000;
	INTX.IPRH.WORD = 0x0000;
	INTX.IPRI.WORD = 0x0000;
	INTX.IPRJ.WORD = 0x0000;
	INTX.IPRK.WORD = 0x0000;
	INTX.IPRL.WORD = 0x0000;

	// Saving keyboard registers.
	inj1	= *((volatile unsigned short *)0xa4050116);
	data1	= *((volatile unsigned char  *)0xa4050136);
	inj2	= *((volatile unsigned short *)0xa4050118);
	data2	= *((volatile unsigned char  *)0xa4050138);
	det	= *((volatile unsigned short *)0xa405014c);
	keys	= *((volatile unsigned char  *)0xa405016c);
	reg	= *((volatile unsigned char  *)0xa40501c6);

	// Allowing RTC. Keyboard analysis is done regularly using a RTC
	// because SH7305's special KEYSC interface does not allow us to clear
	// the keyboard interrupt flags.
	INTX.IPRK._RTC		= GINT_INTP_RTC;
	INTX.IPRA.TMU0_0	= GINT_INTP_KEY;
	INTX.IPRA.TMU0_1	= GINT_INTP_GRAY;
	INTX.IPRA.TMU0_2	= GINT_INTP_TIMER;
}

static void gint_priority_unlock_7305(void)
{
	// Restoring the interrupt priorities.
	INTX.IPRA.WORD = ipr[0];
	INTX.IPRB.WORD = ipr[1];
	INTX.IPRC.WORD = ipr[2];
	INTX.IPRD.WORD = ipr[3];
	INTX.IPRE.WORD = ipr[4];
	INTX.IPRF.WORD = ipr[5];
	INTX.IPRG.WORD = ipr[6];
	INTX.IPRH.WORD = ipr[7];
	INTX.IPRI.WORD = ipr[8];
	INTX.IPRJ.WORD = ipr[9];
	INTX.IPRK.WORD = ipr[10];
	INTX.IPRL.WORD = ipr[11];

	// Restoring keyboard registers.
	*((volatile unsigned short *)0xa4050116) = inj1;
	*((volatile unsigned char  *)0xa4050136) = data1;
	*((volatile unsigned short *)0xa4050118) = inj2;
	*((volatile unsigned char  *)0xa4050138) = data2;
	*((volatile unsigned short *)0xa405014c) = det;
	*((volatile unsigned char  *)0xa405016c) = keys;
	*((volatile unsigned char  *)0xa40501c6) = reg;
}

void gint_setup_7305(void)
{
	gint_priority_lock_7305();

	// Saving the RTC configuration.
	rcr2 = RTC.RCR2.BYTE;
	// Disabling RTC interrupts by default.
	RTC.RCR2.BYTE = 0x09;
}

void gint_stop_7305(void)
{
	gint_priority_unlock_7305();

	// Restoring the RTC configuration.
	RTC.RCR2.BYTE = rcr2;
}
