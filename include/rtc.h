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
	Defines a point in time. This structure *is* the standard struct tm to
	avoid useless data copy in the interface between this module and the
	standard time module.
*/
struct RTCTime
{
	int seconds;		// Seconds in range 0-59
	int minutes;		// Minutes in range 0-59
	int hours;		// Hours in range 0-23
	int month_day;		// Day of month in range 1-31
	int month;		// Month in range 0-11
	int year;		// Number of years since 1900
	int week_day;		// Day of week in range 0(Sunday)-6(Saturday).
	int year_day;		// Day of the year in range 0-365.
	int daylight_saving;	// As far as I known the RTC does not use DST.
};

/*
	rtc_getTime()
	Reads the current time from the RTC.
*/
struct RTCTime rtc_getTime(void);

/*
	rtc_setTime()
	Sets the time in the RTC registers.
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
};

/*
	rtc_setCallback()
	Sets the callback function for the real-time clock interrupt. If
	frequency is non-zero, the clock frequency is set to the given value.
	Set the callback to NULL to deactivate an existing callback.
*/
void rtc_setCallback(void (*callback)(void), enum RTCFrequency frequency);

/*
	rtc_getCallback()
	Returns the callback function. If frequency is non-NULL, it is set to
	the current frequency value.
*/
void (*rtc_getCallback(enum RTCFrequency *frequency))(void);



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
	rtc_setFrequency()
	Sets the RTC interrupt frequency and enables interrupts.
*/
void rtc_setFrequency_7705(enum RTCFrequency frequency);
void rtc_setFrequency_7305(enum RTCFrequency frequency);

/*
	rtc_getFrequency()
	Returns the RTC interrupt frequency.
*/
enum RTCFrequency rtc_getFrequency_7705(void);
enum RTCFrequency rtc_getFrequency_7305(void);

#endif	// _RTC_H
