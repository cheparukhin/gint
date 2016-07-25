#include <7705.h>
#include <rtc.h>

//---
//	Various MPU-dependent procedures.
//---

/*
	rtc_setFrequency()
	Sets the RTC interrupt frequency and enables interrupts.
*/
void rtc_setFrequency_7705(enum RTCFrequency frequency)
{
	if(frequency < 1 || frequency > 7) return;
	RTC.RCR2.BYTE = (frequency << 4) | 0x09;
}

/*
	rtc_getFrequency()
	Returns the RTC interrupt frequency.
*/
enum RTCFrequency rtc_getFrequency_7705(void)
{
	return (RTC.RCR2.BYTE & 0x70) >> 4;
}

/*
	rtc_interrupt()
	Handles an RTC interrupt by calling the callback.
*/
void rtc_interrupt_7705(void)
{
	RTC.RCR2.BIT.PEF = 0;
}
