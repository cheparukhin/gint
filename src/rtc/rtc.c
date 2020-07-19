//---
//	gint:rtc - Real-Time Clock
//---

#include <gint/rtc.h>
#include <gint/drivers.h>
#include <gint/gint.h>
#include <gint/intc.h>

#include <gint/defs/types.h>
#include <gint/hardware.h>
#include <gint/mpu/rtc.h>

#include <stdarg.h>

#undef rtc_start_timer

//---
//	Real-Time Clock peripheral registers
//---

/* RTC address on SH7305, adjusted at startup on SH7337 and SH7355 */
static rtc_t *RTC = &SH7305_RTC;
/* Address of interrupt handler parameters */
GBSS static struct {
	void *function;
	uint32_t arg;
} GPACKED(4) *timer_params;

//---
//	Time management
//---

/* int8(), int16(): Convert BCD to integer */
static int int8(uint8_t bcd)
{
	return (bcd & 0x0f) + 10 * (bcd >> 4);
}
static int int16(uint16_t bcd)
{
	return (bcd & 0xf) + 10 * ((bcd >> 4) & 0xf) + 100 * ((bcd >> 8) & 0xf)
		+ 1000 * (bcd >> 12);
}

/* bcd8(), bcd16(): Convert integer to BCD
   TODO: Use some kind of qdiv() for bcd8() and bcd16() */
static uint8_t bcd8(int integer)
{
	integer %= 100;
	return ((integer / 10) << 4) | (integer % 10);
}
static uint16_t bcd16(int integer)
{
	integer %= 10000;
	return (bcd8(integer / 100) << 8) | bcd8(integer % 100);
}

/* rtc_get_time(): Read the current time from the RTC */
void rtc_get_time(rtc_time_t *time)
{
	do {
		RTC->RCR1.CF = 0;

		time->seconds   = int8(RTC->RSECCNT.byte);
		time->minutes   = int8(RTC->RMINCNT.byte);
		time->hours     = int8(RTC->RHRCNT.byte);
		time->month_day = int8(RTC->RDAYCNT.byte);
		time->month     = int8(RTC->RMONCNT.byte);
		time->year      = int16(RTC->RYRCNT.word);
		time->week_day  = RTC->RWKCNT;

	} while(RTC->RCR1.CF != 0);
}

/* rtc_set_time(): Set current time in the RTC */
void rtc_set_time(rtc_time_t const *time)
{
	int wday = time->week_day;
	if(wday >= 7) wday = 0;

	do {
		RTC->RCR1.CF = 0;

		RTC->RSECCNT.byte = bcd8(time->seconds);
		RTC->RMINCNT.byte = bcd8(time->minutes);
		RTC->RHRCNT.byte  = bcd8(time->hours);
		RTC->RDAYCNT.byte = bcd8(time->month_day);
		RTC->RMONCNT.byte = bcd8(time->month);
		RTC->RYRCNT.word  = bcd16(time->year);
		RTC->RWKCNT       = wday;

	} while(RTC->RCR1.CF != 0);
}

//---
//	RTC timer
//---

/* rtc_start_timer(): Configure the RTC timer */
int rtc_start_timer(int freq, timer_callback_t f, ...)
{
	/* Refuse to override an existing callback */
	if(RTC->RCR2.PES != RTC_NONE) return 1;
	if(freq == RTC_NONE) return 1;

	/* Get the argument */
	va_list args;
	va_start(args, f);
	uint32_t arg = va_arg(args, uint32_t);
	va_end(args);

	/* Temporarily disable the interrupt and set up the callback */
	RTC->RCR2.PES = RTC_NONE;
	timer_params->function = f.v;
	timer_params->arg = arg;

	/* Clear the interrupt flag */
	do RTC->RCR2.PEF = 0;
	while(RTC->RCR2.PEF);

	/* Enable the interrupt */
	RTC->RCR2.PES = freq;
	return 0;
}

/* rtc_stop_timer(): Stop the RTC timer */
void rtc_stop_timer(void)
{
	RTC->RCR2.PES = RTC_NONE;
}

//---
//	Driver initialization
//---

#if defined(FX9860G) || (!defined(FX9860G) && !defined(FXCG50))
static void driver_sh3(void)
{
	/* Adjust the address of the RTC */
	RTC = &SH7705_RTC;
}
#endif

static void init(void)
{
	/* Disable the carry and alarm interrupts (they share their IPR bits
	   with the periodic interrupt, which we want to enable) */
	RTC->RCR1.byte = 0;
	/* Clear the periodic interrupt flag */
	RTC->RCR2.PEF = 0;

	/* Interrupt handlers provided by rtc/inth.s */
	extern void inth_rtc_pri(void);
	extern void inth_rtc_pri_helper(void);

	/* Install the RTC interrupt handler */
	GUNUSED void *h0, *h1;
	h0 = gint_inthandler(0xaa0, inth_rtc_pri, 32);
	h1 = gint_inthandler(0xae0, inth_rtc_pri_helper, 32);

	timer_params = h0 + 20;

	volatile uint8_t **RCR2_pointer = h1 + 28;
	*RCR2_pointer = &RTC->RCR2.byte;

	/* Disable the RTC interrupts for now. Give them priority 1; higher
	   priorities cause freezes when going back to the system on SH3
	   (TODO: Find out about the RTC interrupt problem on SH3) */
	RTC->RCR2.PES = RTC_NONE;
	intc_priority(INTC_RTC_PRI, 1);
}

//---
//	Context system for this driver
//---

typedef struct
{
	uint8_t RCR1;
	uint8_t RCR2;
} ctx_t;

GBSS static ctx_t sys_ctx, gint_ctx;

static void ctx_save(void *buf)
{
	ctx_t *ctx = buf;
	ctx->RCR1 = RTC->RCR1.byte;
	ctx->RCR2 = RTC->RCR2.byte;
}

static void ctx_restore(void *buf)
{
	ctx_t *ctx = buf;

	RTC->RCR1.byte = ctx->RCR1 & 0x18;
	RTC->RCR2.byte = ctx->RCR2 & 0x7f;
}

//---
//	Driver structure definition
//---

gint_driver_t drv_rtc = {
	.name         = "RTC",
	.driver_sh3   = GINT_DRIVER_SH3(driver_sh3),
	.init         = init,
	.sys_ctx      = &sys_ctx,
	.gint_ctx     = &gint_ctx,
	.ctx_save     = ctx_save,
	.ctx_restore  = ctx_restore,
};

GINT_DECLARE_DRIVER(2, drv_rtc);
