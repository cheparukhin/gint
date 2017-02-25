#ifndef	_INTERNALS_TIMER_H
#define	_INTERNALS_TIMER_H	1

/*
	struct Timer
	This structure holds information for a running timer.
*/
struct Timer
{
	void *callback;
	void *data;
	int repeats;
};

extern struct Timer timers[3];

/*
	struct mod_tmu
	This structure holds information about the timer unit (peripheral
	module) registers.
*/
struct mod_tmu
{
	unsigned int TCOR;	// Timer constant register.
	unsigned int TCNT;	// Timer counter.

	union
	{
		unsigned short WORD;
		struct
		{
			unsigned	:7;
			unsigned UNF	:1;	// Underflow flag.
			unsigned	:2;
			unsigned UNIE	:1;	// Underflow interrupt enable.
			unsigned CKEG	:2;	// Clock edge (SH7705 only).
			unsigned TPSC	:3;	// Timer prescaler.
		};
	} TCR;	// Timer control register.
};

/*
	timer_get()
	Returns the timer and TSTR register addresses.
*/
void timer_get(int timer, volatile struct mod_tmu **tmu,
	volatile unsigned char **tstr);

#endif	// _INTERNALS_TIMER_H
