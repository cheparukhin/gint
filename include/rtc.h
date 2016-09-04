//---
//
//	gint core module: rtc
//
//	Manages RTC. This module is used behind standard module time.
//
//---

#ifndef	_RTC_H
#define	_RTC_H	1

//---
//	Time access.
//---

/*
	struct RTCTime
	Defines a point in time.
*/
struct RTCTime
{
	int seconds;		// Seconds in range 0-59
	int minutes;		// Minutes in range 0-59
	int hours;		// Hours in range 0-23
	int month_day;		// Day of month in range 1-31
	int month;		// Month in range 0-11
	int year;		// Years (full value)
	int week_day;		// Day of week in range 0(Sunday)-6(Saturday).
};

/*
	rtc_getTime()
	Reads the current time from the RTC. There is no guarantee that the
	week day is correct (use the time API for that).
*/
struct RTCTime rtc_getTime(void);

/*
	rtc_setTime()
	Sets the time in the RTC registers. The week day is set to 0 if greater
	than 6. Other fields are not checked.
*/
void rtc_setTime(struct RTCTime time);



//---
//	Callback API.
//---

/*
	enum RTCFrequency
	Describes the possible frequencies available for the real-time clock
	interrupt.
*/
enum RTCFrequency
{
	RTCFreq_500mHz	= 7,
	RTCFreq_1Hz	= 6,
	RTCFreq_2Hz	= 5,
	RTCFreq_4Hz	= 4,
	RTCFreq_16Hz	= 3,
	RTCFreq_64Hz	= 2,
	RTCFreq_256Hz	= 1,
	RTCFreq_None	= 0,
};

/*
	rtc_cb_add()
	Registers a new callback for the RTC. Returns the callback id on
	success (positive integer), or one of the following error codes:
	  -1	Array is full
	  -2	Invalid parameter
	The number of repeats may be set to 0, in which case the callback is
	called indefinitely unless the user calls rtc_cb_end().
*/
int rtc_cb_add(enum RTCFrequency freq, void (*function)(void), int repeats);

/*
	rtc_cb_end()
	Removes the callback with the given id (as returned by rtc_cb_add())
	from the callback array.
*/
void rtc_cb_end(int id);

/*
	rtc_cb_edit()
	Changes information related to a callback. This function returns 0 on
	success, or one of the following error codes:
	  -1	Callback does not exist
	  -2	Invalid parameters
	This function never removes a callback. Call rtc_cb_end() for this. One
	can set the function to NULL or the frequency to RTCFreq_None to
	temporarily disable the callback.
*/
int rtc_cb_edit(int id, enum RTCFrequency new_freq,
	void (*new_function)(void));



//---
//	Internal API.
//	Referenced here for documentation purposes only. Do not call.
//---

/*
	rtc_interrupt()
	Handles an RTC interrupt by calling the callback.
*/
void rtc_interrupt(void)		__attribute__((section(".gint.int")));
void rtc_interrupt_7705(void)		__attribute__((section(".gint.int")));
void rtc_interrupt_7305(void)		__attribute__((section(".gint.int")));

/*
	rtc_cb_interrupt()
	Handles an RTC interrupt. Calls the RTC callbacks if necessary, and
	updates the repeat counts.
*/
void rtc_cb_interrupt(void);

#endif	// _RTC_H
