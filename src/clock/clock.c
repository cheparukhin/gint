#include <clock.h>
#include <timer.h>
#include <internals/timer.h>
#include <rtc.h>
#include <stddef.h>
#include <mpu.h>

static struct ClockConfig conf = {
	.FLL = -1, .PLL = -1,
	.Bphi_div1 = -1, .Iphi_div1 = -1, .Pphi_div1 = -1,
	.CKIO_f = -1,
	.Bphi_f = -1, .Iphi_f = -1, .Pphi_f = -1
};

/*
	clock_frequency()
	Returns the approximate frequency, in Hz, of the given clock. The
	measurements need to have been done. Returns a negative number on
	error.
*/
int clock_frequency(enum Clock clock)
{
	switch(clock)
	{
	case Clock_CKIO:
		return conf.CKIO_f;
	case Clock_RTCCLK:
		return conf.RTCCLK_f;
	case Clock_Bphi:
		return conf.Bphi_f;
	case Clock_Iphi:
		return conf.Iphi_f;
	case Clock_Pphi:
		return conf.Pphi_f;

	default:
		return -2;
	}
}

/*
	clock_setting()
	Returns the P_phi / 4 timer setting that will last for the given time.
	Several units can be used. Be aware that the result is approximate, and
	very high frequencies or very short delays will yield important errors.
*/
int clock_setting(int duration, enum ClockUnit unit)
{
	if(conf.Pphi_f <= 0) return -1;
	int f = conf.Pphi_f >> 2;

	switch(unit)
	{
	case Clock_us:
		return (duration * f) / 1000000;
	case Clock_ms:
		return (duration * f) / 1000;
	case Clock_s:
		return (duration * f);

	case Clock_Hz:
		return f / duration;
	case Clock_kHz:
		return f / (duration * 1000);
	case Clock_MHz:
		return f / (duration * 1000000);

	default:
		return -1;
	}
}

/*
	clock_config()
	Returns a copy of the clock configuration.
*/
struct ClockConfig clock_config(void)
{
	return conf;
}

/*
	sleep()
	Sleeps until an interrupt is accepted.
*/
void sleep(void)
{
	__asm__(
		"sleep\n\t"
	);
}

/*
	sleep_us()
	Sleeps for the given number of us using the user timer. The result will
	always be slightly less than required.
*/

static volatile int sleep_us_done = 0;

static void sleep_us_callback(void)
{
	sleep_us_done = 1;
}

void sleep_us(int us_delay)
{
	sleep_us_done = 0;
	timer_start(TIMER_USER, clock_setting(us_delay, Clock_us), TIMER_Po_4,
		sleep_us_callback, 1);
	do sleep();
	while(!sleep_us_done);
}



//---
//	Clock frequency measurements -- Public API.
//---

// Indicates whether the measurements are finished.
static volatile int clock_measure_done = 0;
// Once again SH7705 and SH7305 need different methods...
static int cb_id_7705 = -1;
static void clock_measure_7705();
static void clock_compute_7305();

/*
	clock_measure()
	Measures or computes the clock frequencies.
*/
void clock_measure(void)
{
	// On SH7705 we cannot have the value of CKIO simply, so we measure
	// P_phi using a timer/RTC combination, and we deduce CKIO.
	if(isSH3())
	{
		// We prepare the timer manually, without starting it, so that
		// we only have to push the running bit to start it when the
		// measurements begin. This might look of little effect but it
		// makes the precision jump from ~97% to more than 99%.
		volatile struct mod_tmu *tmu;
		timer_get(TIMER_USER, &tmu, NULL);

		tmu->TCOR	= 0xffffffff;
		tmu->TCNT	= tmu->TCOR;
		tmu->TCR.TPSC	= TIMER_Po_4;

		tmu->TCR.UNF	= 0;
		tmu->TCR.UNIE	= 1;
		tmu->TCR.CKEG	= 0;

		timers[TIMER_USER].callback	= NULL;
		timers[TIMER_USER].repeats	= 0;

		cb_id_7705 = rtc_cb_add(RTCFreq_256Hz, clock_measure_7705, 0);
	}

	// On SH7305, assuming clock mode 3, we can compute the clock
	// frequencies because we know that RTC_CLK oscillates at 32768 Hz.
	else
	{
		clock_compute_7305();
		clock_measure_done = 1;
	}
}

/*
	clock_measure_end()
	Waits until the measurements are finished. This may be immediate.
*/
void clock_measure_end(void)
{
	while(!clock_measure_done) sleep();
}

//---
//	Clock frequency measurements -- SH7305.
//---

/*
	clock_compute_7305()
	Computes the clock frequencies according to the CPG parameters.
*/
static void clock_compute_7305(void)
{
	volatile unsigned int *FRQCRA	= (void *)0xa4150000;
	volatile unsigned int *PLLCR	= (void *)0xa4150024;
	volatile unsigned int *FLLFRQ	= (void *)0xa4150050;

	// Surely the documentation of SH7724 does not meet the specification
	// of SH7305 for the PLL setting, because the register accepts other
	// values than the ones specified for SH7724. The relation given by
	// Sentaro21 (thanks again!) yields good results.
	int pll = (*FRQCRA >> 24) & 0x3f;	// Raw setting
	pll = pll + 1;				// Resulting multiplier
	conf.PLL = pll;

	// This one is simpler. The FLL ratio is actually the setting value.
	int fll = *FLLFRQ & 0x7ff;		// Raw setting = multiplier
	if(*FLLFRQ & (1 << 14)) fll >>= 1;	// Halve-output flag
	conf.FLL = fll;

	// The divider1 ratios are NOT those of SH7724. The relation between
	// the values below and the divider ratios is given by Sentaro21
	// (thanks to him!) and satisfies ratio = 1 / (2 ** (setting + 1)).
	int div1_bphi = (*FRQCRA >>  8) & 0xf;
	int div1_iphi = (*FRQCRA >> 20) & 0xf;
	int div1_pphi = (*FRQCRA      ) & 0xf;

	conf.Bphi_div1 = 1 << (div1_bphi + 1);
	conf.Iphi_div1 = 1 << (div1_iphi + 1);
	conf.Pphi_div1 = 1 << (div1_pphi + 1);

	// Computing the frequency of the signal, which is input to divider 1.
	int base = 32768;
	if(*PLLCR & (1 << 12)) base *= fll;
	if(*PLLCR & (1 << 14)) base *= pll;

	conf.RTCCLK_f	= 32768;
	conf.Bphi_f	= base >> (div1_bphi + 1);
	conf.Iphi_f	= base >> (div1_iphi + 1);
	conf.Pphi_f	= base >> (div1_pphi + 1);
}

//---
//	Clock frequency measurements -- SH7705.
//---

/*
	clock_measure_7705_finalize()
	Given the number of P_phi / 4 timer ticks elapsed between two RTC
	256 Hz interrupts, determines the clock configuration.
*/
static void clock_measure_7705_finalize(int elapsed)
{
	volatile unsigned int *FRQCR	= (void *)0xffffff80;

	conf.Pphi_f	= elapsed * 4 * 256;
	if(conf.Pphi_f <= 0) return;

	conf.PLL1	= ((*FRQCR >> 8) & 0x03) + 1;
	conf.PLL2	= -1;

	conf.Bphi_div1	= 0;
	conf.Iphi_div1	= ((*FRQCR >> 4) & 0x03) + 1;
	conf.Pphi_div1	= ((*FRQCR     ) & 0x03) + 1;

	conf.CKIO_f	= (conf.Pphi_f * conf.Pphi_div1) / conf.PLL1;
	conf.Bphi_f	= conf.CKIO_f;
	conf.Iphi_f	= (conf.CKIO_f * conf.PLL1) / conf.Iphi_div1;
}

/*
	clock_measure_7705_callback()
	Starts measurements. Measurements will end automatically. Do not use
	RTC interrupt or the user timer will doing measurements.
	Call clock_measure_end() when you need to use those, to ensure
	measurements are finished.
*/
static void clock_measure_7705_callback(void)
{
	timer_stop(TIMER_USER);
	rtc_cb_end(cb_id_7705);

	volatile struct mod_tmu *tmu;
	timer_get(TIMER_USER, &tmu, NULL);
	int elapsed = 0xffffffff - tmu->TCNT;

	clock_measure_7705_finalize(elapsed);
	clock_measure_done = 1;
}

/*
	clock_measure_7705()
	Programs the clock measurements. We need to have the user timer and the
	RTC synchronized for this operation, so we wait for an RTC interrupt
	and we prepare the timer beforehand to avoid losing processor time in
	configuring the registers.
*/
static void clock_measure_7705(void)
{
	volatile unsigned char *tstr;
	timer_get(TIMER_USER, NULL, &tstr);

	*tstr |= (1 << TIMER_USER);
	rtc_cb_edit(cb_id_7705, RTCFreq_256Hz, clock_measure_7705_callback);
}
