//---
//
//	gint core module: timer
//
//	Basic timer unit manipulation. Starts and stops timers with variable
//	number of repeats, and allows timer reloads without pause.
//
//---

#ifndef	_TIMER_H
#define	_TIMER_H

#include <clock.h>
#include <stdint.h>
#include <stddef.h>

// The timer object is manipulated by the module; the user needs not access it
// directly. Its contents are defined in <internals/timer.h>.
struct timer_t;
typedef struct timer_t timer_t;

//---
//	Virtual timer API
//	Gint allows users to create virtual timers with 1-ms precision to
//	handle an important amount of timed events. The amount of virtual
//	timers available is gapped by the TIMER_SLOTS parameter, but this
//	parameter can be customized when building the library; thus, the
//	amount of usable virtual timers is not limited.
//---

#ifndef	TIMER_SLOTS
#define	TIMER_SLOTS	16
#endif

/*
	timer_create()
	Basic timer configuration. This function creates a virtual timer and
	configures its delay and repetition count. It returns a virtual timer
	object that will be used by other functions. At most TIMER_SLOTS
	virtual timers can be used at the same time: this function returns NULL
	if there is no new slot available.
	By default a virtual timer configured by timer_configure() will fire
	ET_Timer events, which the user will need to catch. The user should
	then execute some action.
	There is another option: the user may call timer_attach() to attach a
	callback to a virtual timer. A virtual timer which has a callback
	attached will not fire any ET_Timer event and will call the callback
	automatically instead.
*/
timer_t *timer_create(int delay_ms, int repeats);

/*
	timer_reload()
	Changes a virtual timer's delay. The timer is not stopped nor started:
	it keeps running or waiting. Events that were waiting to be handled are
	dropped and the number of repeats left is not changed. The timer
	restarts counting from 0 regardless of how much time had elapsed since
	it last fired.
*/
void timer_reload(timer_t *timer, int new_ms_delay);

/*
	timer_destroy()
	Destroys a virtual timer. This virtual timer pointer becomes invalid
	and should not be used anymore.
*/
void timer_destroy(timer_t *virtual_timer);



//---
//	Hardware timer API
//	Gint provides access to hardware timer. These timer offer more or less
//	microsecond-level control. However, the user should ensure, when using
//	hardware timers, that they are not overriding the configuration of
//	timers that are already running -- gint won't check.
//---

/*
	timer_hard_t
	Available hardware timers. The user can use timer_user freely, but
	timer_gray and timer_virtual should not be used as long as the gray
	engine or virtual timers are running (respectively).
*/
typedef enum
{
	timer_tmu0	= 0,
	timer_virtual	= timer_tmu0,

	timer_tmu1	= 1,
	timer_gray	= timer_tmu1,

	timer_tmu2	= 2,
	timer_user	= timer_tmu2,

} timer_hard_t;

/*
	timer_input_t
	Available input clocks for the hardware timer:
	  - Po/4	Peripheral clock (frequency divided by 4)
	  - Po/16	Peripheral clock (frequency divided by 16)
	  - Po/64	Peripheral clock (frequency divided by 64)
	  - Po/256	Peripheral clock (frequency divided by 256)
	  - TCLK	External clock
	I'm not totally sure there is any signal on the external clock, so
	don't use it unless you know what you are doing.
*/
typedef enum
{
	timer_Po_4	= 0,
	timer_Po_16	= 1,
	timer_Po_64	= 2,
	timer_Po_256	= 3,
	timer_tclk	= 5,

} timer_input_t;

/*
	htimer_setup()
	Configures a hardware timer. By default hardware timers generates
	ET_Timer events but catching them may take some time, especially if
	there are other events waiting in the queue. For increased timing, and
	for fast timers, the user should consider using callbacks instead.
	Returns a hardware timer object.
	Returns the correct timer structure if the requested timer is free and
	NULL otherwise.
*/
timer_t *htimer_setup(timer_hard_t id, uint32_t constant, timer_input_t input,
	int repeats);

/*
	htimer_reload()
	Reloads a hardware timer without starting or stopping it.
*/
void htimer_reload(timer_hard_t id, uint32_t new_constant);



//---
//	Common API
//	The following functions work with both virtual and hardware timers.
//---

/*
	timer_attach()
	This function attaches a callback to a virtual or hardware timer. A
	timer with a callback attached will stop firing ET_Timer events and
	will call the callback function directly instead.
	The type signature of the function should be as follows:
	  - void callback(void)            if argument == NULL
	  - void callback(void *argument)  if argument is non-NULL
	In the latter case, the argument pointer will be passed to the callback
	function.
*/
void timer_attach(timer_t *timer, void *callback, void *argument);

/*
	timer_start()
	Starts a virtual or hardware timer. If the timer has a callback
	attached, then the callback function will start being called regularly;
	otherwise, the timer will start pushing ET_Timer events to the event
	queue. It is advised, not to change a timer's configuration while it's
	running.
*/
void timer_start(timer_t *timer);

/*
	timer_stop()
	Stops a timer. If the argument is virtual timer, it is paused and can
	be started again later.
	If it's a hardware timer, it is freed and made accessible to other
	uses. It should be set up again before being started.
*/
void timer_stop(timer_t *timer);

#endif // _TIMER_H
