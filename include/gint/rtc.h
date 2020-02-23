//---
//	gint:rtc - Real-Time Clock
//---

#ifndef GINT_RTC
#define GINT_RTC

#include <gint/defs/types.h>

//---
//	Time management
//---

/* rtc_time_t - a point in time, representable in the RTC registers */
typedef struct
{
	uint16_t year;		/* Years (exact value, e.g. 2018) */
	uint8_t week_day;	/* Day of week, (0=Sunday, 6=Saturday) */
	uint8_t month;		/* Month (0..11) */
	uint8_t month_day;	/* Day of month (1..31) */
	uint8_t hours;		/* Hour (0..23) */
	uint8_t minutes;	/* Minute (0..59) */
	uint8_t seconds;	/* Second (0..59) */

} rtc_time_t;

/* rtc_get_time() - read the current time from the RTC
   @time  Pointer to rtc_time_t structure (needs not be initialized) */
void rtc_get_time(rtc_time_t *time);

/* rtc_set_time() - write a new current time to the RTC
   If [time->week_day] is not in the valid range, it is set to 0. Other fields
   are not checked.
   @time  Pointer to new time */
void rtc_set_time(const rtc_time_t *time);

//---
//	RTC timer
//	The real-time clock produces a regular interrupt which may be used as a
//	timer with a maximum frequency of 256 Hz. It is also useful to check
//	that the clock settings (see <gint/clock.h>) are properly detected, by
//	comparing the detected frequencies with the RTC.
//---

/* rtc_frequency_t - possible frequency settings for the RTC's interrupt */
typedef enum
{
	RTC_500mHz	= 7,
	RTC_1Hz		= 6,
	RTC_2Hz		= 5,
	RTC_4Hz		= 4,
	RTC_16Hz	= 3,
	RTC_64Hz	= 2,
	RTC_256Hz	= 1,
	RTC_NONE	= 0,

} rtc_frequency_t;

/* rtc_start_timer() - configure the RTC timer

   This function sets up the RTC timer to invoke the provided callback function
   with its argument regularly. This works like normal timers (<gint/timer.h>);
   [callback] is passed [arg] as argument and the RTC timer is stopped if it
   returns non-zero.

   This function will replace any existing callback!

   Note that, as opposed to timers, it is not possible to know how much time
   will elapse before the callback will first be called. For instance, setting
   up a 1 Hz-callback when the current time ends in 950 ms will result in the
   callback being called after 50 ms, then every second. This is not a problem
   for most uses. */
void rtc_start_timer(rtc_frequency_t freq,
	int (*callback)(volatile void *arg), volatile void *arg);

/* rtc_stop_timer() - stop the RTC timer
   This function stops the RTC timer that was set up with rtc_start_timer(). If
   the decision of stopping the timer comes from the callback, it is preferable
   to return non-zero. */
void rtc_stop_timer(void);

#endif /* GINT_RTC */
