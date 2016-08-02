#include <internals/timer.h>
#include <mpu.h>

/*
	timer_get()
	Returns the timer and TSTR register addresses.
*/
void timer_get(int timer, volatile struct mod_tmu **tmu,
	volatile unsigned char **tstr)
{
	// Using SH7705 information for SH-3-based MPUs.
	if(isSH3())
	{
		if(tstr) *tstr = (volatile unsigned char *)0xfffffe92;
		if(tmu) *tmu = (volatile struct mod_tmu *)
			(0xfffffe94 + 12 * timer);
	}
	// Assuming SH7305 by default.
	else
	{
		if(tstr) *tstr = (volatile unsigned char *)0xa4490004;
		if(tmu) *tmu = (volatile struct mod_tmu *)
			(0xa4490008 + 12 * timer);
	}
}
