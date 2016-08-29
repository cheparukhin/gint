//---
//
//	gint core module: clock
//
//	Measures the frequency of the MPU clocks. This module assumes that the
//	clock mode is 3 on SH7305 (as does FTune).
//
//---

//---
//	Some type declarations.
//---

enum Clock
{
	Clock_CKIO 	= 0,	// SH7705
	Clock_RTCCLK	= 1,	// SH7305
	Clock_Bphi	= 2,
	Clock_Iphi	= 3,
	Clock_Pphi	= 4,
};

enum ClockUnit
{
	Clock_us	= 0,
	Clock_ms	= 1,
	Clock_s		= 2,

	Clock_Hz	= 10,
	Clock_kHz	= 11,
	Clock_MHz	= 12,
};

struct ClockConfig
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
};

//---
//	Public API.
//---

/*
	clock_frequency()
	Returns the approximate frequency, in Hz, of the given clock. The
	measurements need to have been done. Returns a negative number on
	error.
*/
int clock_frequency(enum Clock clock);

/*
	clock_setting()
	Returns the P_phi / 4 timer setting that will last for the given time.
	Several units can be used. Be aware that the result is approximate, and
	very high frequencies or very short delays will yield important errors.
*/
int clock_setting(int duration, enum ClockUnit unit);

/*
	clock_config()
	Returns a copy of the clock configuration.
*/
struct ClockConfig clock_config(void);

/*
	sleep()
	Sleeps until an interrupt is accepted.
*/
void sleep(void);

/*
	sleep_us()
	Sleeps for the given number of us using the user timer. The result will
	always be slightly less than required.
*/
void sleep_us(int us_delay);



//---
//	Internal API.
//	Referenced for documentation purposes only. Do not use.
//---

/*
	clock_measure()
	Begins the frequency measurements. The measurements will end
	automatically. While doing measurements, do not use the RTC interrupt
	or the user timer.
	Call clock_measure_end() to wait until the measurements are finished.
	It is possible to execute code during the measurements, so that less
	time is spent.
*/
void clock_measure(void);

/*
	clock_measure_end()
	Waits until the measurements are finished. This may be immediate.
*/
void clock_measure_end(void);
