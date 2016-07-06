#include <gint.h>
#include <timer.h>
#include <keyboard.h>
#include <7305.h>

//---
//	Various MPU-dependent procedures.
//---

/*
	gint_setRTCFrequency()
	Sets the RTC interrupt frequency and enables interrupts.
*/
void gint_setRTCFrequency_7305(enum RTCFrequency frequency)
{
	if(frequency < 1 || frequency > 7) return;
	RTC.RCR2.BYTE = (frequency << 4) | 0x09;
}

/*
	gint_getRTCFrequency()
	Returns the RTC interrupt frequency.
*/
enum RTCFrequency gint_getRTCFrequency_7305(void)
{
	return (RTC.RCR2.BYTE & 0x70) >> 4;
}
