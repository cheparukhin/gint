#include <timer_internals.h>
#include <mpu.h>

/*
	timer_get()
	Returns the timer and TSTR register addresses.
*/
void timer_get(int timer, struct mod_tmu **tmu, unsigned char **tstr)
{
	// Using SH7705 information for SH-3-based MPUs.
	if(isSH3())
	{
		if(tstr) *tstr = (unsigned char *)0xfffffe92;
		if(tmu) *tmu = (struct mod_tmu *)0xfffffe94;
	}
	// Assuming SH7305 by default.
	else
	{
		if(tstr) *tstr = (unsigned char *)0xa4490004;
		if(tmu) *tmu = (struct mod_tmu *)0xa4490008;
	}

	// Shifting tmu value to get to the timer-nth timer in the unit.
	if(tmu) *tmu += timer;
}
