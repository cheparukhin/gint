#include <rtc.h>
#include <mpu.h>

#include <stddef.h>

static void (*rtc_callback)(void) = NULL;

//---
//	Callback API.
//---

/*
	rtc_setCallback()
	Sets the callback function for the real-time clock interrupt. If
	frequency is non-NULL, the clock frequency is set to the given value.
*/
void rtc_setCallback(void (*callback)(void), enum RTCFrequency frequency)
{
	rtc_callback = callback;
	if(frequency < 1 || frequency > 7) return;

	if(isSH3())
		rtc_setFrequency_7705(frequency);
	else
		rtc_setFrequency_7305(frequency);
}

/*
	rtc_getCallback()
	Returns the callback function. If frequency is non-NULL, it is set to
	the current frequency value.
*/
void (*rtc_getCallback(enum RTCFrequency *frequency))(void)
{
	if(!frequency) return rtc_callback;

	if(isSH3())
		*frequency = rtc_getFrequency_7705();
	else
		*frequency = rtc_getFrequency_7305();

	return rtc_callback;
}



//---
//	Interrupt management.
//---

/*
	rtc_interrupt()
	Handles an RTC interrupt by calling the callback.
*/
void rtc_interrupt(void)
{
	if(rtc_callback) (*rtc_callback)();

	if(isSH3())
		rtc_interrupt_7705();
	else
		rtc_interrupt_7305();
}
