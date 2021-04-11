#include <gint/usb.h>
#include <gint/mpu/usb.h>
#include <gint/mpu/power.h>
#include <gint/mpu/cpg.h>
#include <gint/drivers.h>
#include <gint/clock.h>
#include <gint/intc.h>
#include <gint/gint.h>
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
static gint_callback_t usb_open_callback = GINT_CB_NULL;
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

static bool usb_module_active(void)
{
	return (SH7305_CPG.USBCLKCR.CLKSTP == 0) &&
		(SH7305_POWER.MSTPCR2.USB0 == 0);
}

/* Start the clock of the USB module, without enabling operations or notifying
   the host. This procedure does just enough to allow reading registers, and
   writing to SYSCFG and BUSWAIT. If (enable_write == true), it also turns on
   SCKE so that registers can be written to. */
static void usb_module_start(bool enable_write)
{
	if(!usb_module_active())
	{
		/* TODO: USB: Proper handling of MSELCRA and MSELCRB */
		uint16_t volatile *MSELCRA = (void *)0xa4050180;
		uint16_t volatile *MSELCRB = (void *)0xa4050182;
		*MSELCRA &= 0xff3f;
		*MSELCRB &= 0x3fff;

		/* Leave some delay for the clock to settle (like OS). I have
		   observed that after a reset, 20 ms are *NOT ENOUGH* for the
		   clock to start properly. 35 ms seemed fine. (?) */
		SH7305_CPG.USBCLKCR.CLKSTP = 0;
		sleep_us_spin(50000);

		SH7305_POWER.MSTPCR2.USB0 = 0;
		SH7305_USB_UPONCR.word = 0x0600;
	}

	if(!enable_write) return;

	/* Turn on SCKE, which activates all other registers. Because the clock
	   for the USB module is 48 MHz and the processor runs at a higher
	   frequency, wait for a little bit before modifying registers. Writing
	   within 3-4 CPU cycles is 100% unsafe and has been observed to cause
	   freezes during testing. */
	USB.SYSCFG.SCKE = 1;
	for(int i = 0; i < 10; i++) __asm__ volatile("nop");

	/* Set BUSWAIT to a safe value, hopefully avoiding overlock problems */
	USB.BUSWAIT.word = 15;
}

/* Stop the clock of the USB module. */
static void usb_module_stop(bool restore_mselcr)
{
	uint16_t volatile *MSELCRA = (void *)0xa4050180;
	uint16_t volatile *MSELCRB = (void *)0xa4050182;

	SH7305_USB_UPONCR.word = 0x0000;

	/* This delay is crucial and omitting it has caused constant freezes in
	   the past. Blame undocumented clock magic? */
	sleep_us_spin(1000);
	SH7305_POWER.MSTPCR2.USB0 = 1;

	SH7305_CPG.USBCLKCR.CLKSTP = 1;
	sleep_us_spin(50000);

	if(!restore_mselcr) return;

	/* The values used by the OS (a PFC driver could do better) */
	*MSELCRB = (*MSELCRB & 0x3fff) | 0xc000;
	*MSELCRA = (*MSELCRA & 0xff3f) | 0x0040;
}

int usb_open(usb_interface_t const **interfaces, gint_callback_t callback)
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
	usb_module_start(true);

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

	gint_inthandler(0xa20, inth_usb, 32);
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
	usb_module_stop(false);
	usb_log("---- usb_close ----\n");

	usb_open_callback = GINT_CB_NULL;
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
				gint_callback_invoke(usb_open_callback);
				usb_open_callback = GINT_CB_NULL;
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
// Hardware context
//---

typedef struct
{
	/* Control and power-up. We don't try to save power-related registers
	   from other modules nor UPONCR, because (1) they have to be changed
	   to access the module, so the OS cannot realy on their value being
	   preserved, and (2) numerous timing and order issues mean in practice
	   that changing them less makes program more stable. */
	uint16_t SYSCFG, DVSTCTR, TESTMODE, REG_C2;
	/* FIFO configuration */
	uint16_t CFIFOSEL, D0FIFOSEL, D1FIFOSEL;
	/* Interrupt configuration */
	uint16_t INTENB0, BRDYENB, NRDYENB, BEMPENB, SOFCFG;
	/* Default Control Pipe (maybe not needed) */
	uint16_t DCPCFG, DCPMAXP, DCPCTR;
	/* Whether the module is running at this time */
	bool active;
} ctx_t;

GBSS static ctx_t sys_ctx, gint_ctx;

static void ctx_save(void *buf)
{
	ctx_t *ctx = buf;

	/* We only save the OS context once to avoid unnecessary quick power
	   cycles (which are the most unstable things in this driver) */
	static bool has_saved_sys = false;
	if(buf == &sys_ctx && has_saved_sys) return;
	if(buf == &sys_ctx) has_saved_sys = true;

	ctx->active = usb_module_active() || (ctx == &gint_ctx);

	/* Power ON the USB module clock so that the registers can be saved.
	   We don't enable writing since we only want to observe registers */
	usb_module_start(false);

	ctx->SYSCFG    = USB.SYSCFG.word;
	ctx->DVSTCTR   = USB.DVSTCTR.word;
	ctx->TESTMODE  = USB.TESTMODE.word;
	ctx->REG_C2    = USB.REG_C2;

	ctx->CFIFOSEL  = USB.CFIFOSEL.word;
	ctx->D0FIFOSEL = USB.D0FIFOSEL.word;
	ctx->D1FIFOSEL = USB.D1FIFOSEL.word;

	ctx->INTENB0   = USB.INTENB0.word;
	ctx->BRDYENB   = USB.BRDYENB.word;
	ctx->NRDYENB   = USB.NRDYENB.word;
	ctx->BEMPENB   = USB.BEMPENB.word;
	ctx->SOFCFG    = USB.SOFCFG.word;

	ctx->DCPCFG    = USB.DCPCFG.word;
	ctx->DCPMAXP   = USB.DCPMAXP.word;
	ctx->DCPCTR    = USB.DCPCTR.word;

	/* Leave the module open for gint to use it, or for the next restore
	   to proceed more quickly (if during a world switch) */
}

static void ctx_restore(void *buf)
{
	ctx_t *ctx = buf;

	usb_module_start(true);

	USB.DVSTCTR.word        = ctx->DVSTCTR;
	USB.TESTMODE.word       = ctx->TESTMODE;
	USB.REG_C2              = ctx->REG_C2;

	USB.CFIFOSEL.word       = ctx->CFIFOSEL;
	USB.D0FIFOSEL.word      = ctx->D0FIFOSEL;
	USB.D1FIFOSEL.word      = ctx->D1FIFOSEL;

	USB.INTENB0.word        = ctx->INTENB0;
	USB.BRDYENB.word        = ctx->BRDYENB;
	USB.NRDYENB.word        = ctx->NRDYENB;
	USB.BEMPENB.word        = ctx->BEMPENB;
	USB.SOFCFG.word         = ctx->SOFCFG;

	USB.DCPCFG.word         = ctx->DCPCFG;
	USB.DCPMAXP.word        = ctx->DCPMAXP;
	USB.DCPCTR.word         = ctx->DCPCTR;

	/* Clear remaining interrupts. Read-only bits will be ignored */
	USB.INTSTS0.word        = 0;
	USB.BRDYSTS.word        = 0;
	USB.NRDYSTS.word        = 0;
	USB.BEMPSTS.word        = 0;

	/* Restore SYSCFG last as clearing SCKE disables writing */
	USB.SYSCFG.word         = ctx->SYSCFG;

	if(!ctx->active) usb_module_stop(true);
}

//---
// Driver structure definition
//---

gint_driver_t drv_usb = {
	.name         = "USB",
	.sys_ctx      = &sys_ctx,
	.gint_ctx     = &gint_ctx,
	.ctx_save     = ctx_save,
	.ctx_restore  = ctx_restore,
};

GINT_DECLARE_DRIVER(3, drv_usb);
