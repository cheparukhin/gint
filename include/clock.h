//---
//
//	gint core module: clock
//
//	Measures the frequency of the MPU clocks. This module assumes that the
//	clock mode is 3 on SH7305 (as does FTune).
//
//---

#ifndef	_CLOCK_H
#define	_CLOCK_H

//---
//	Sleep functions.
//---

/*
	sleep()
	Puts the processor to sleep until an interrupt request is issued.
*/
void sleep(void);

/*
	sleep_ms(), sleep_us()
	Sleeps for the given number of milliseconds / microseconds using the
	TIMER_USER timer. The actual sleep time will always be slightly less
	than requested.
*/
void sleep_ms(int ms_delay);
void sleep_us(int us_delay);



//---
//	Clock management.
//---

enum ClockUnit
{
	Clock_us	= 0,
	Clock_ms	= 1,
	Clock_s		= 2,

	Clock_Hz	= 10,
	Clock_kHz	= 11,
	Clock_MHz	= 12,
};

typedef struct
{
	union
	{
		int PLL1;	// SH7705
		int FLL;	// SH7305
	};
	union
	{
		int PLL2;	// SH7705
		int PLL;	// SH7305
	};

	int Bphi_div1;
	int Iphi_div1;
	int Pphi_div1;

	union
	{
		int CKIO_f;	// SH7705
		int RTCCLK_f;	// SH7305
	};

	int Bphi_f;
	int Iphi_f;
	int Pphi_f;

} clock_config_t;

/*
	clock_setting()
	Returns the P_phi / 4 timer setting that will last for the given time.
	Several units can be used. Be aware that the result is approximate, and
	very high frequencies or very short delays will yield important errors.
	Normally you need not use this function when setting up timers because
	timer_start() handles this conversion for you.
*/
int clock_setting(int duration, enum ClockUnit unit);

/*
	clock_config()
	Returns a copy of what the library knows about the clocks.
*/
clock_config_t clock_config(void);

#endif	// _CLOCK_H
