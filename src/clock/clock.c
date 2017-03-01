#include <clock.h>
#include <timer.h>
#include <modules/timer.h>
#include <rtc.h>
#include <stddef.h>
#include <mpu.h>
#include <stdint.h>

static clock_config_t conf = {
	.FLL = -1, .PLL = -1,
	.Bphi_div1 = -1, .Iphi_div1 = -1, .Pphi_div1 = -1,
	.CKIO_f = -1,
	.Bphi_f = -1, .Iphi_f = -1, .Pphi_f = -1
};

/*
	clock_setting()
	Returns the P_phi / 4 timer setting that will last for the given time.
	Several units can be used. Be aware that the result is approximate, and
	very high frequencies or very short delays will yield important errors.
*/
uint32_t clock_setting(int duration, enum ClockUnit unit)
{
	if(conf.Pphi_f <= 0) return 0xffffffff;
	uint64_t f = conf.Pphi_f >> 2;
	uint64_t result;

	switch(unit)
	{
	case Clock_us:
		result = (duration * f) / 1000000;
		break;
	case Clock_ms:
		result = (duration * f) / 1000;
		break;
	case Clock_s:
		result = (duration * f);
		break;

	case Clock_Hz:
		result = f / duration;
		break;
	case Clock_kHz:
		result = f / (duration * 1000);
		break;
	case Clock_MHz:
		result = f / (duration * 1000000);
		break;

	default:
		return -1;
	}

	return (result > 0xffffffff) ? (0xffffffff) : (result);
}

/*
	clock_config()
	Returns a copy of the clock configuration.
*/
clock_config_t clock_config(void)
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
		"sleep"
	);
}

static void sleep_callback(void *arg)
{
	int *flag = arg;
	*flag = 1;
}

/*
	sleep_ms()
	Sleeps for the given number of milliseconds using a virtual timer.
*/
void sleep_ms(int ms_delay)
{
	volatile int sleep_done = 0;

	timer_t *timer = timer_create(ms_delay, 1);
	timer_attach(timer, sleep_callback, (void *)&sleep_done);
	timer_start(timer);

	while(!sleep_done) sleep();
}

/*
	sleep_us()
	Sleeps for the given number of microseconds using the hardware timer
	timer_user.
*/
void sleep_us(int us_delay)
{
	volatile int sleep_done = 0;
	const uint32_t constant = clock_setting(us_delay, Clock_us);

	timer_t *timer = htimer_setup(timer_user, constant, timer_Po_4, 1);
	timer_attach(timer, sleep_callback, (void *)&sleep_done);
	timer_start(timer);

	while(!sleep_done) sleep();
}



//---
//	Clock frequency measurements -- Public API.
//---

// Indicates whether the measurements are finished.
static volatile int clock_measure_done = 0;
// Once again SH7705 and SH7305 need different methods...
static timer_t *htimer_7705 = NULL;
static int cb_id_7705 = -1;
static void clock_measure_7705(void);
static void clock_compute_7305(void);

/*
	clock_measure()
	Begins the frequency measurements. The measurements will end
	automatically. While doing measurements, do not use the RTC interrupt
	or the user timer.
	Call clock_measure_end() to wait until the measurements are finished.
	It is possible to execute code during the measurements, so that less
	time is spent.
*/
void clock_measure(void)
{
	// On SH7705 we cannot have the value of CKIO simply, so we measure
	// P_phi using a timer/RTC combination, and we deduce CKIO.
	if(isSH3())
	{
		htimer_7705 = htimer_setup(timer_user, 0xffffffff, timer_Po_4,
			1);
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
static void clock_measure_7705_finalize(uint32_t elapsed)
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
	timer_stop(htimer_7705);
	rtc_cb_end(cb_id_7705);

	uint32_t elapsed = 0xffffffff - TMU.timers[timer_user]->TCNT;
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
	timer_start(htimer_7705);
	rtc_cb_edit(cb_id_7705, RTCFreq_256Hz, clock_measure_7705_callback);
}
