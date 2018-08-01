//---
//	gint:timer - Timer operation
//---

#ifndef GINT_TIMER
#define GINT_TIMER

#include <defs/attributes.h>
#include <defs/types.h>
#include <core/mpu.h>

/* Timer identifiers

   Hardware timers are numbered with integers starting from 0. You can freely
   access all the available timers by using their number once you have
   configured them with timer_setup(). The number of timers depends on the MPU:

     SH3-based: 4 timers, ids 0..3  [SH7355, SH7337]
     SH4-based: 9 timers, ids 0..8  [SH7305]

   You should be aware that some of these timers are used by default by gint:
   - Timer 1 is used by the gray engine on fx9860g.
   - Timer 3 is used by the keyboard, unless GINT_RTC_KEYBOARD is defined. This
     macro is controlled by the -rtc-keyboard switch when gint is compiled.

   timer_setup() will fail if you try to use a timer that's already running.
   Always check the return value of timer_setup()! Using a timer id that has
   not been validated by timer_setup() will work, but do *something else* than
   what you intended. */

/* timer_count() - tells how many timers are available on the platform */
HDRFUNC int timer_count(void)
{
	mpuSwitch(
		return 4,	/* SH3-based */
		return 9	/* SH4-based */
	);
}

/* Clock input

   Timers count down when their input clock ticks, and fire when their counter
   reach 0. The choice of the input clock influences the resolution of the
   timer, but if the clock is too fast, the 32-bit counter might not be able to
   represent long delays.

   Several input clocks are available. The peripheral clock (Po) can be divided
   by 4, 16, 64 or 256; as an alternative the external clock TCLK can be used
   for counting. I suspect TCLK runs at a fixed frequency of 32768 Hz, but this
   has yet to be verified.

   You don't really need to choose an input clock unless you are doing
   something very specific. In most practical cases you can use timer_default
   which is 0. See the timer_delay() function for more information. */
typedef enum
{
	timer_Po_4	= 0,
	timer_Po_16	= 1,
	timer_Po_64	= 2,
	timer_Po_256	= 3,
	timer_TCLK	= 5,

	timer_default	= timer_Po_4,

} timer_input_t;

//---
//	Timer functions
//---

/* timer_setup() - set up a timer

   This function configures the requested timer without starting it. On
   success, it returns the first argument "timer", which is used as a timer
   identifier in all other timer functions. If the requested timer is already
   in use, this function fails and returns a negative number.

   This function sets the timer delay, the clock source, and registers a
   callback function to be called when the timer fires. An argument can be
   supplied to the callback function in the form of a pointer.

   When the timer fires, the callback function is called with the provided
   argument pointer. The callback decides whether the timer should continue
   running (by returning 0) or stop (by returning nonzero). In the latter case,
   events accumulated while the callback was running are dropped.

   It is sometimes difficult to choose a timer constant and a clock source
   given a wished delay in seconds, especially when overclock is used. The
   timer_delay() function is provided for this purpose.

   @timer     Requested timer id
   @delay     Delay between each event (the unit depends on the clock source)
   @clock     Clock source used by the timer for counting down
   @callback  Callback function (called when the timer fires)
   @arg       Passed as argument to the callback function */
int timer_setup(int timer, uint32_t delay, timer_input_t clock,
	int (*callback)(void *arg), void *arg);

/* timer_delay() - compute a delay constant from a duration in seconds

   This function can used as a facility to calculate the [delay] argument to
   the timer_setup() function. It takes a microsecond delay as an argument and
   returns the corresponding timer constant. A typical use to start a timer
   with a 25 ms interval would be:

     timer_setup(0, timer_delay(0, 25 * 1000), 0, callback, arg);

   WARNING: Only timers 0 to 2 can count microseconds! Other timers have a
   resolution of around 30 us. Counting in ms is safe for all timers, though.

   For standard timers (0 to 2) it uses Po / 4 as clock input, which is very
   precise and can represent up to 3 minutes' time; for extra timers (3 and
   above) the clock is fixed to 32768 Hz.

   @timer     The timer you are planning to use
   @delay_us  Requested delay in microseconds */
uint32_t timer_delay(int timer, int delay_us);

/* timer_start() - start a configured timer
   The specified timer will start counting down and fire callbacks at regular
   intervals.

   @timer  Timer id, as returned by timer_setup() */
void timer_start(int timer);

/* timer_reload() - change a timer's delay and source

   Changes the delay and source of the given timer. This function does not
   start the timer if it stopped. The timer will start counting down from the
   new delay when it is started (or immediately if it was running), no matter
   how much time has elapsed since it last fired. Accumulated events are not
   dropped.

   In cases where choosing a delay and input clock is difficult (such as when
   using overclocking), use the timer_delay() function.

   @timer  Timer id, as returned by timer_setup()
   @delay  New delay (unit depends on the clock source)
   @clock  New clock source to be used by the timer */
void timer_reload(int timer, uint32_t delay, timer_input_t clock);

/* timer_pause() - stop a running timer
   The specified timer will be paused; its counter will not be reset. A stopped
   timer can be resumed anytime by calling timer_start(). If you want to also
   reset the counter, use timer_reload().

  @timer  Timer id, as returned by timer_setup() */
void timer_pause(int timer);

/* timer_free() - free a timer
   Stops and destroys a timer, making its id free for re-use. The id must not
   be used anymore until it is returned by a further call to timer_setup().

   @timer  Timer id, as returned by timer_setup() */
void timer_free(int timer);

#endif /* GINT_TIMER */
