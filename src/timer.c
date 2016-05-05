#include <timer.h>
#include <mpu.h>
#include <stddef.h>

//---
//	Internal declarations.
//	Timer structure and running  information (callbacks, repeats etc.)
//---

/*
	struct Timer
	This structure holds information for a running timer.
*/
struct Timer
{
	void (*callback)(void);
	int repetitions;
};

// Static timers.
static struct Timer timers[3] = { { NULL, 0 }, { NULL, 0 }, { NULL, 0 } };

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



//---
//	Internal API.
//---

/*
	timer_get()

	Returns the timer and TSTR register addresses.

	@arg	timer	Timer id.
	@arg	tmu	mod_tmu structure pointer address.
	@arg	tstr	mod_tstr structure pointer address.
*/
static void timer_get(int timer, struct mod_tmu **tmu, unsigned char **tstr)
{
	// Using SH7705 information for SH-3-based MPUs.
	if(MPU_CURRENT == MPU_SH7337 || MPU_CURRENT == MPU_SH7355)
	{
		if(tstr) *tstr = (unsigned char *)0xfffffe92;
		if(tmu) *tmu = (struct mod_tmu *)0xfffffe94;
	}
	// Assuming SH7305 by default.
	else
	{
		if(tstr) *tstr = (unsigned char *)0xa4490004;
		if(tmu) *tmu = (struct mod_tmu *)0xa4490008;
	}

	// Shifting tmu value to get to the timer-nth timer in the unit.
	if(tmu) *tmu += timer;
}

/*
	timer_interrupt()
	Handles the interrupt for the given timer.

	@timer	Timer that generated the interrupt.
*/
void timer_interrupt(int timer)
{
	// Getting the timer address.
	struct mod_tmu *tmu;
	timer_get(timer, &tmu, NULL);

	// Resetting the interrupt flag.
	(*tmu).TCR.UNF = 0;

	// Calling the callback function.
	if(timers[timer].callback) timers[timer].callback();

	// Reducing the number of repetitions left, if not infinite.
	if(!timers[timer].repetitions) return;
	// And stopping it if necessary.
	if(timers[timer].repetitions == 1) timer_stop(timer);
	else timers[timer].repetitions--;
}



//---
//	Public API.
//---

/*
	timer_start()
	Configures and starts a timer.

	@arg	timer		Timer identifier.
	@arg	delay		Delay before expiration.
	@arg	prescaler	Clock prescaler value.
	@arg	callback	Callback function.
	@arg	repetitions	Number of repetitions, 0 for infinite.
*/
void timer_start(int timer, int delay, int prescaler, void (*callback)(void),
	int repetitions)
{
	// Getting the timer address. Using a byte to alter TSTR.
	struct mod_tmu *tmu;
	unsigned char *tstr;
	int byte = (1 << timer);
	timer_get(timer, &tmu, &tstr);

	// Setting the constant register.
	(*tmu).TCOR = delay;
	// Loading the delay in the counter.
	(*tmu).TCNT = delay;

	// Resetting underflow flag.
	(*tmu).TCR.UNF = 0;
	// Enabling interruptions on underflow.
	(*tmu).TCR.UNIE = 1;
	// Counting on rising edge. On SH7305 these two bits are reserved but
	// writing 0 is ignored.
	(*tmu).TCR.CKEG = 0;
	// Setting the prescaler.
	(*tmu).TCR.TPSC = prescaler;

	// Loading the structure information.
	timers[timer].callback = callback;
	timers[timer].repetitions = repetitions;

	// Starting the timer and returning.
	*tstr |= byte;
}

/*
	timer_stop()
	Stops the given timer. This function may be called even if the timer is
	not running.

	@arg	timer	Timer to stop.
*/
void timer_stop(int timer)
{
	// Getting TSTR address and the corresponding byte.
	unsigned char *tstr;
	int byte = (1 << timer);
	timer_get(timer, NULL, &tstr);

	// Stopping the timer.
	*tstr &= ~byte;
}
