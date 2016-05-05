#ifndef	_TIMER_H
#define	_TIMER_H	1

//---
//	Constants.
//---

// Timer identifiers.
#define	TIMER_0		0
#define TIMER_TMU0	TIMER_0
#define	TIMER_1		1
#define TIMER_TMU1	TIMER_1
#define	TIMER_2		2
#define	TIMER_TMU2	TIMER_2
// Timer function identifiers.
#define	TIMER_KEYBOARD	TIMER_TMU0
#define	TIMER_GRAY	TIMER_TMU1
#define	TIMER_USER	TIMER_TMU2

// Timer prescalers.
#define	TIMER_Po_4	0
#define	TIMER_Po_16	1
#define TIMER_Po_64	2
#define TIMER_Po_256	3
#define TIMER_TCLK	5



//---
//	Public API.
//---

/*
	timer_start()
	Configures and starts a timer.

	@arg	timer		Timer name. Use only TIMER_USER. You may use
				TIMER_GRAY, if you're not running the gray
				engine.
	@arg	delay		Delay before expiration, in clock counts.
	@arg	prescaler	Clock prescaler value. Possible values are
				TIMER_Po_4, TIMER_Po_16, TIMER_Po_64,
				TIMER_Po_256 and TIMER_TCLK.
	@arg	callback	Callback function.
	@arg	repetitions	Number of repetitions, 0 for infinite.
*/
void timer_start(int timer, int delay, int prescaler, void (*callback)(void),
	int repetitions);

/*
	timer_stop()
	Stops the given timer. This function may be called even if the timer is
	not running.

	@arg	timer	Timer identifier.
*/
void timer_stop(int timer);



//---
//	Internal API.
//	Referenced for documentation purposes only. Do not call.
//---

/*
	timer_interrupt()
	Handles the interrupt for the given timer.

	@timer	Timer that generated the interrupt.
*/
void timer_interrupt(int timer)		__attribute__((section(".gint.int")));

#endif // _TIMER_H
