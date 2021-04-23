#include <gint/usb.h>
#include <gint/mpu/usb.h>
#include <gint/mpu/power.h>
#include <gint/mpu/cpg.h>
#include <gint/drivers.h>
#include <gint/drivers/states.h>
#include <gint/clock.h>
#include <gint/intc.h>
#include "usb_private.h"

#define USB SH7305_USB

/* Interrupt handler */
extern void inth_usb(void);

/* Shorthand to clear a bit in INTSTS0 */
#define INTSTS0_clear(field_name) {				\
	__typeof__(USB.INTSTS0) __intsts0 = { .word = 0xffff };	\
	__intsts0.field_name = 0;				\
	do USB.INTSTS0 = __intsts0;				\
	while(USB.INTSTS0.field_name != 0);			\
}

/* Callback function to invoke when the USB module is configured */
/* TODO: usb_open() callback: Let interfaces specify when they're ready! */
static gint_call_t usb_open_callback = GINT_CALL_NULL;
/* Whether the USB link is currently open */
static bool volatile usb_open_status = false;

//---
// Logging system
//---

static void (*usb_logger)(char const *format, va_list args) = NULL;

void usb_set_log(void (*logger)(char const *format, va_list args))
{
	usb_logger = logger;
}

void usb_log(char const *format, ...)
{
	if(!usb_logger) return;

	va_list args;
	va_start(args, format);
	usb_logger(format, args);
	va_end(args);
}

//---
// Module powering and depowering
//---

static bool hpowered(void)
{
	return (SH7305_CPG.USBCLKCR.CLKSTP == 0) &&
		(SH7305_POWER.MSTPCR2.USB0 == 0);
}

static void hpoweron(void)
{
	if(hpowered()) return;

	/* TODO: USB: Proper handling of MSELCRA and MSELCRB */
	uint16_t volatile *MSELCRA = (void *)0xa4050180;
	uint16_t volatile *MSELCRB = (void *)0xa4050182;
	*MSELCRA &= 0xff3f;
	*MSELCRB &= 0x3fff;

	/* Leave some delay for the clock to settle. The OS leaves
	   100 ms, but it just never seems necessary. */
	SH7305_CPG.USBCLKCR.CLKSTP = 0;
	sleep_us_spin(1000);

	SH7305_POWER.MSTPCR2.USB0 = 0;
	SH7305_USB_UPONCR.word = 0x0600;

	/* Turn on SCKE, which activates all other registers. The existing
	   BUSWAIT delay might not be high enough, so wait a little bit before
	   modifying registers; a couple CPU cycles is enough. */
	USB.SYSCFG.SCKE = 1;
	for(int i = 0; i < 10; i++) __asm__ volatile("nop");

	/* Set BUSWAIT to a safe value, hopefully avoiding overlock problems */
	USB.BUSWAIT.word = 15;
}

static void hpoweroff(void)
{
	uint16_t volatile *MSELCRA = (void *)0xa4050180;
	uint16_t volatile *MSELCRB = (void *)0xa4050182;

	SH7305_USB_UPONCR.word = 0x0000;

	/* This delay is crucial and omitting it has caused constant freezes in
	   the past. Blame undocumented clock magic? */
	sleep_us_spin(1000);
	SH7305_POWER.MSTPCR2.USB0 = 1;

	SH7305_CPG.USBCLKCR.CLKSTP = 1;
	sleep_us_spin(1000);

	/* The values used by the OS (a PFC driver could do better) */
	*MSELCRB = (*MSELCRB & 0x3fff) | 0xc000;
	*MSELCRA = (*MSELCRA & 0xff3f) | 0x0040;
}

int usb_open(usb_interface_t const **interfaces, gint_call_t callback)
{
	usb_log("---- usb_open ----\n");

	int rc = usb_configure_solve(interfaces);
	usb_configure_log();

	if(rc != 0)
	{
		usb_log("configure failure: %d\n", rc);
		return rc;
	}

	usb_open_callback = callback;
	if(!hpowered()) hpoweron();

	*(uint16_t volatile *)0xa4d800c2 = 0x0020;

	/* Turn on the module now that SCKE=1 */
	USB.SYSCFG.USBE = 1;
	USB.SYSCFG.HSE = 1;

	/* Disable pin pull-down and pull-up in order to change DCFM */
	USB.SYSCFG.DRPD = 0;
	USB.SYSCFG.DPRPU = 0;
	/* Select function controller */
	USB.SYSCFG.DCFM = 0;

	/* Pull D+ up to 3.3V, notifying connection when possible. Read
	   SYSSTS.LNST as required after modifying DPRPU */
	USB.SYSCFG.DPRPU = 1;
	GUNUSED volatile int LNST = USB.SYSSTS.LNST;

	/* Prepare the default control pipe. */
	USB.DCPCFG.DIR = 0;
	USB.DCPMAXP.DEVSEL = 0;
	USB.DCPMAXP.MXPS = 64;

	USB.SOFCFG.enable = 1;

	/* Configure CFIFOSEL to use the DCP */
	USB.CFIFOSEL.RCNT = 0;
	USB.CFIFOSEL.REW = 0;
	USB.CFIFOSEL.BIGEND = 1;
	/* Clear D0FIFOSEL and D1FIFOSEL so that pipes can be configured */
	USB.D0FIFOSEL.word = 0x0000;
	USB.D1FIFOSEL.word = 0x0000;

	/* Configure other pipes to use activated interfaces */
	usb_configure();
	/* Initialize transfer tracker for multi-part transfers */
	usb_pipe_init_transfers();

	/* VBSE=1 RSME=0 SOFE=0 DVSE=1 CTRE=1 BEMPE=1 NRDYE=0 BRDYE=0 */
	USB.INTENB0.word = 0x9c00;
	USB.BRDYENB.word = 0x0000;
	USB.NRDYENB.word = 0x0000;
	USB.BEMPENB.word = 0x0000;

	intc_handler(0xa20, inth_usb, 32);
	intc_priority(INTC_USB, 15);

	usb_open_status = true;
	return 0;
}

void usb_open_wait(void)
{
	while(!usb_open_status) sleep();
}

void usb_close(void)
{
	intc_priority(INTC_USB, 0);
	hpoweroff();
	usb_log("---- usb_close ----\n");

	usb_open_callback = GINT_CALL_NULL;
	usb_open_status = false;
}

//---
// Userspace interrupt handler
//---

void usb_interrupt_handler(void)
{
	static char const * const device_st[] = {
		"powered", "default", "address", "configured",
		"suspended-powered", "suspended-default", "suspended-address",
		"suspended-configured",
	};
	/* Save PIPESEL to avoid concurrent access issues */
	uint16_t pipesel = USB.PIPESEL.word;
	/* Change D0FIFOSEL to make sure that the same pipe can't be specified
	   in D0FIFOSEL and D1FIFOSEL simultaneously */
	uint16_t d0fifosel = USB.D0FIFOSEL.word;
	USB.D0FIFOSEL.word = 0x0000;

	if(USB.INTSTS0.VBINT)
	{
		INTSTS0_clear(VBINT);
		usb_log("VBUS %s\n", USB.INTSTS0.VBSTS ? "up" : "down");
	}
	else if(USB.INTSTS0.CTRT)
	{
		INTSTS0_clear(CTRT);
		if(USB.INTSTS0.VALID) usb_req_setup();
	}
	else if(USB.INTSTS0.DVST)
	{
		INTSTS0_clear(DVST);
		usb_log("DVST %s", device_st[USB.INTSTS0.DVSQ]);
		if(USB.INTSTS0.DVSQ == 2) usb_log(": %04x\n",USB.USBADDR.word);
		else usb_log("\n");

		/* When configured, run the callback for usb_open() */
		if(USB.INTSTS0.DVSQ == 3)
		{
			usb_open_status = true;
			if(usb_open_callback.function)
			{
				gint_call(usb_open_callback);
				usb_open_callback = GINT_CALL_NULL;
			}
		}
	}
	else if(USB.INTSTS0.BEMP)
	{
		/* Invoke callbacks for each buffer-empty interrupt */
		uint16_t status = USB.BEMPSTS.word;
		USB.BEMPSTS.word = 0;

		for(int i = 1; i <= 9; i++)
		{
			if(status & (1 << i)) usb_pipe_write_bemp(i);
		}
	}
	else usb_log("<%04X> -> ???\n", USB.INTSTS0.word);

	/* Disable D1FIFO so that concurrent access does not occur */
	USB.D1FIFOSEL.word = 0x0000;
	USB.D0FIFOSEL.word = d0fifosel;
	/* Restore PIPESEL which can have been used for transfers */
	USB.PIPESEL.word = pipesel;
}

//---
// State and driver metadata
//---

void hsave(usb_state_t *s)
{
	s->SYSCFG    = USB.SYSCFG.word;
	s->DVSTCTR   = USB.DVSTCTR.word;
	s->TESTMODE  = USB.TESTMODE.word;
	s->REG_C2    = USB.REG_C2;

	s->CFIFOSEL  = USB.CFIFOSEL.word;
	s->D0FIFOSEL = USB.D0FIFOSEL.word;
	s->D1FIFOSEL = USB.D1FIFOSEL.word;

	s->INTENB0   = USB.INTENB0.word;
	s->BRDYENB   = USB.BRDYENB.word;
	s->NRDYENB   = USB.NRDYENB.word;
	s->BEMPENB   = USB.BEMPENB.word;
	s->SOFCFG    = USB.SOFCFG.word;

	s->DCPCFG    = USB.DCPCFG.word;
	s->DCPMAXP   = USB.DCPMAXP.word;
	s->DCPCTR    = USB.DCPCTR.word;

	/* Leave the module open for gint to use it, or for the next restore
	   to proceed more quickly (if during a world switch) */
}

static void hrestore(usb_state_t const *s)
{
	USB.DVSTCTR.word        = s->DVSTCTR;
	USB.TESTMODE.word       = s->TESTMODE;
	USB.REG_C2              = s->REG_C2;

	USB.CFIFOSEL.word       = s->CFIFOSEL;
	USB.D0FIFOSEL.word      = s->D0FIFOSEL;
	USB.D1FIFOSEL.word      = s->D1FIFOSEL;

	USB.INTENB0.word        = s->INTENB0;
	USB.BRDYENB.word        = s->BRDYENB;
	USB.NRDYENB.word        = s->NRDYENB;
	USB.BEMPENB.word        = s->BEMPENB;
	USB.SOFCFG.word         = s->SOFCFG;

	USB.DCPCFG.word         = s->DCPCFG;
	USB.DCPMAXP.word        = s->DCPMAXP;
	USB.DCPCTR.word         = s->DCPCTR;

	/* Clear remaining interrupts. Read-only bits will be ignored */
	USB.INTSTS0.word        = 0;
	USB.BRDYSTS.word        = 0;
	USB.NRDYSTS.word        = 0;
	USB.BEMPSTS.word        = 0;

	/* Restore SYSCFG last as clearing SCKE disables writing */
	USB.SYSCFG.word         = s->SYSCFG;
}

gint_driver_t drv_usb = {
	.name         = "USB",
	.hpowered     = hpowered,
	.hpoweron     = hpoweron,
	.hpoweroff    = hpoweroff,
	.hsave        = (void *)hsave,
	.hrestore     = (void *)hrestore,
	.state_size   = sizeof(usb_state_t),
};
GINT_DECLARE_DRIVER(16, drv_usb);
