#ifndef	_TIMER_INTERNALS_H
#define	_TIMER_INTERNALS_H

/*
	struct Timer
	This structure holds information for a running timer.
*/
struct Timer
{
	void (*callback)(void);
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
	// Timer constant register.
	unsigned int TCOR;
	// Timer counter.
	unsigned int TCNT;

	// Timer control register.
	union
	{
		unsigned short WORD;
		struct
		{
			unsigned	:7;
			// Underflow flag.
			unsigned UNF	:1;
			unsigned	:2;
			// Underflow interrupt enable.
			unsigned UNIE	:1;
			// Clock edge, reserved on SH7305.
			unsigned CKEG	:2;
			// Timer prescaler.
			unsigned TPSC	:3;
		};
	} TCR;
};

/*
	timer_get()
	Returns the timer and TSTR register addresses.
*/
void timer_get(int timer, struct mod_tmu **tmu, unsigned char **tstr);

#endif	// _TIMER_INTERNALS_H
