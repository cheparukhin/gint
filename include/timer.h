//---
//
//	gint core module: timer
//
//	Basic timer unit manipulation. Starts and stops timers with variable
//	number of repeats, and allows timer reloads without pause.
//
//---

#ifndef	_TIMER_H
#define	_TIMER_H	1

#include <clock.h>

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
	Configures and starts a timer. The timer argument expects a timer name.
	You can use TIMER_USER anytime. You may also use TIMER_GRAY if you're
	not running the gray engine.
	The delay is in clock counts unit. The possible values for the
	prescaler are dividers of the peripheral clock frequency Po:
	- TIMER_Po_4
	- TIMER_Po_16
	- TIMER_Po_64
	- TIMER_Po_256
	- TIMER_TCLK
	The number of repeats may to set to 0. In this case, the timer will not
	stop until timer_stop() is explicitly called.
*/
void timer_start(int timer, int delay, int prescaler, void (*callback)(void),
	int repeats);

/*
	timer_stop()
	Stops the given timer. This function may be called even if the timer is
	not running.
*/
void timer_stop(int timer);

/*
	timer_reload()
	Reloads the given timer with the given constant. Starts the timer if
	it was stopped. The new delay uses the same unit as in timer_start().
*/
void timer_reload(int timer, int new_delay);



//---
//	Internal API.
//	Referenced for documentation purposes only. Do not call.
//---

/*
	timer_interrupt()
	Handles the interrupt for the given timer.
*/
void timer_interrupt(int timer)		__attribute__((section(".gint.int")));

#endif // _TIMER_H
