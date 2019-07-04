//---
//	gint:r61524 - Renesas R61524 driver
//---

#include <gint/defs/types.h>
#include <gint/hardware.h>
#include <gint/drivers.h>
#include <gint/dma.h>
#include <core/std.h>

#ifdef FXCG50

#define DMA SH7305_DMA
#define POWER SH7305_POWER

//---
//	Device specification sheet
//---

/* Registers and operations */
enum {
	device_code_read		= 0x000,
	driver_output_control		= 0x001,
	entry_mode			= 0x003,
	display_control_2		= 0x008,
	low_power_control		= 0x00b,

	ram_address_horizontal		= 0x200,
	ram_address_vertical		= 0x201,
	write_data			= 0x202,

	horizontal_ram_start		= 0x210,
	horizontal_ram_end		= 0x211,
	vertical_ram_start		= 0x212,
	vertical_ram_end		= 0x213,
};

typedef word_union(entry_mode_t,
	uint TRI	:1;
	uint DFM	:1;
	uint		:1;
	uint BGR	:1;
	uint		:2;
	uint HWM	:1;
	uint		:1;
	uint ORG	:1;
	uint		:1;
	uint ID		:2;
	uint AM		:1;
	uint		:1;
	uint EPF	:2;
);

//---
//	Device communication primitives
//---

#define synco() __asm__ volatile ("synco":::"memory")

/* Interface with the controller */
GDATA static volatile uint16_t *intf = (void *)0xb4000000;
/* Bit 4 of Port R controls the RS bit of the display driver */
GDATA static volatile uint8_t *PRDR = (void *)0xa405013c;

GINLINE static void select(uint16_t reg)
{
	/* Clear RS and write the register number */
	*PRDR &= ~0x10;
	synco();
	*intf = reg;
	synco();

	/* Set RS back. We don't do this in read()/write() because the display
	   driver is optimized for consecutive GRAM access. LCD-transfers will
	   be faster when executing select() followed by several calls to
	   write(). (Although most applications should use the DMA instead.) */
	*PRDR |= 0x10;
	synco();
}

GINLINE static uint16_t read(void)
{
	return *intf;
}

GINLINE static void write(uint16_t data)
{
	*intf = data;
}

//---
//	Window management
//---

void r61524_win_get(uint16_t *HSA, uint16_t *HEA, uint16_t *VSA, uint16_t *VEA)
{
	select(horizontal_ram_start);
	*HSA = read();
	select(horizontal_ram_end);
	*HEA = read();

	select(vertical_ram_start);
	*VSA = read();
	select(vertical_ram_end);
	*VEA = read();
}

void r61524_win_set(uint16_t HSA, uint16_t HEA, uint16_t VSA, uint16_t VEA)
{
	select(horizontal_ram_start);
	write(HSA);
	select(horizontal_ram_end);
	write(HEA);

	select(vertical_ram_start);
	write(VSA);
	select(vertical_ram_end);
	write(VEA);
}

//---
//	Driver functions
//---

/* void r61524_test(void)
{
	uint16_t device_name;
	uint16_t doc;
	int SM, SS;
	entry_mode_t em;
	uint16_t dc2;
	int FP, BP;
	uint16_t lpc;
	int VEM, COL;

	//---

	select(device_code_read);
	device_name = read();

	Bdisp_AllClr_VRAM();
	print(1, 1, "Name=????");
	print_hex(6, 1, device_name, 4);

	if(device_name != 0x1524)
	{
		print(1, 2, "Aborting.");
		Bdisp_PutDisp_DD();
		getkey();
		return;
	}

	//---

	select(driver_output_control);
	doc = read();
	SM = (doc >> 10) & 1;
	SS = (doc >> 8) & 1;

	select(entry_mode);
	em.word = read();

	select(display_control_2);
	dc2 = read();
	FP = (dc2 >> 8) & 0xf;
	BP = dc2 & 0xf;

	select(low_power_control);
	lpc = read();
	VEM = (lpc >> 4) & 1;
	COL = lpc & 1;

	//---

	print(15, 4, " SM=?");
	print_hex(19, 4, SM, 1);
	print(15, 5, " SS=?");
	print_hex(19, 5, SS, 1);

	print(1, 2, "TRI=?  DFM=?  BGR=?");
	print_hex(5,  2, em.TRI, 1);
	print_hex(12, 2, em.DFM, 1);
	print_hex(19, 2, em.BGR, 1);
	print(1, 3, "HWM=?  ORG=?   ID=?");
	print_hex(5,  3, em.HWM, 1);
	print_hex(12, 3, em.DFM, 1);
	print_hex(19, 3, em.ID, 1);
	print(1, 4, " AM=?  EPF=?");
	print_hex(5,  4, em.AM, 1);
	print_hex(12, 4, em.EPF, 1);

	print(1, 5, " FP=?   BP=?");
	print_hex(5, 5, FP, 1);
	print_hex(12, 5, BP, 1);

	print(1, 6, "VEM=?  COL=?");
	print_hex(5, 6, VEM, 1);
	print_hex(12, 6, COL, 1);

	Bdisp_PutDisp_DD();
	getkey();
} */

/* TODO: r61524: update, backlight, brightness, gamma */

void r61524_display(uint16_t *vram, int start, int height, int interrupts)
{
	/* Move the window to the desired region, then select address 0 */
	r61524_win_set(0, 395, start, start + height - 1);
	select(ram_address_horizontal);
	write(0);
	select(ram_address_vertical);
	write(0);

	/* Bind address 0xb4000000 to the data write command */
	select(write_data);

	void *src = vram + 396 * start;
	void *dst = (void *)0xb4000000;

	/* The amount of data sent per row, 396*2, is not a multiple of 32. For
	   now I assume [height] is a multiple of 4, which makes the factor 32
	   appear. */
	int blocks = 99 * (height >> 2);

	/* Now roll! */
	if(interrupts)
	{
		/* Usa a normal, interrupt-based transfer */
		dma_transfer(DMA_32B, blocks, src, DMA_INC, dst, DMA_FIXED);
		/* Wait for it to finish */
		/* TODO: Allow r61524_display() to return early */
		dma_transfer_wait();
	}
	else dma_transfer_noint(DMA_32B, blocks, src, DMA_INC, dst, DMA_FIXED);
}

//---
//	Context system for this driver
//---

typedef struct
{
	/* Graphics RAM range */
	uint16_t HSA, HEA, VSA, VEA;

} GPACKED(2) ctx_t;

/* Allocate one buffer in gint's storage section */
GBSS static ctx_t sys_ctx;

static void ctx_save(void *buf)
{
	ctx_t *ctx = buf;
	r61524_win_get(&ctx->HSA, &ctx->HEA, &ctx->VSA, &ctx->VEA);
}

static void ctx_restore(void *buf)
{
	ctx_t *ctx = buf;
	r61524_win_set(ctx->HSA, ctx->HEA, ctx->VSA, ctx->VEA);
}

//---
//	Driver initialization
//---

static void init(void)
{
	select(device_code_read);
	uint16_t devname = read();

	gint[HWDD] = HW_LOADED | HWDD_FULL;
	if(devname == 0x1524) gint[HWDD] |= HWDD_KNOWN;
}

//---
//	Driver status string
//---

#ifdef GINT_BOOT_LOG

/* r6524_status() - status string */
static const char *r61524_status(void)
{
	select(device_code_read);
	uint16_t dev = read();

	static char str[8];
	sprintf(str, "%4xF-b", dev);
	return str;
}

#endif

//---
//	Driver structure definition
//---

gint_driver_t drv_r61524 = {
	.name		= "R61524",
	.init		= init,
	.status		= GINT_DRIVER_STATUS(r61524_status),
	.ctx_size	= sizeof(ctx_t),
	.sys_ctx	= &sys_ctx,
	.ctx_save	= ctx_save,
	.ctx_restore	= ctx_restore,
};

GINT_DECLARE_DRIVER(5, drv_r61524);

#endif /* FXCG50 */
