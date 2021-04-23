#include <gint/exc.h>
#include <gint/display.h>
#include <gint/clock.h>
#include <gint/mpu/dma.h>
#include <gint/defs/attributes.h>
#include <gint/hardware.h>

#define dprint(x, y, ...) dprint(x, y, C_BLACK, __VA_ARGS__)
#define dtext(x, y, str)  dtext (x, y, C_BLACK, str)

/* gint_panic_default(): Default panic handler */
GNORETURN static void gint_default_panic(GUNUSED uint32_t code)
{
	uint32_t TEA, TRA;

	if(isSH3())
	{
		TEA = *(volatile uint32_t *)0xfffffffc;
		TRA = *(volatile uint32_t *)0xffffffd0 >> 2;
	}
	else
	{
		TEA = *(volatile uint32_t *)0xff00000c;
		TRA = *(volatile uint32_t *)0xff000020 >> 2;
	}

	uint32_t PC;
	__asm__("stc spc, %0" : "=r"(PC));

	uint32_t SGR = 0xffffffff;
	if(isSH4()) __asm__("stc sgr, %0" : "=r"(SGR));

	dfont(NULL);
	dclear(C_WHITE);

	#ifdef FX9860G
	dtext(1, 0, "Exception! (SysERROR)");
	for(int i = 0; i < 32; i++) gint_vram[i] = ~gint_vram[i];

	char const *name = "";
	if(code == 0x040) name = "TLB miss read";
	if(code == 0x060) name = "TLB miss write";
	if(code == 0x0e0) name = "Read address error";
	if(code == 0x100) name = "Write address error";
	if(code == 0x160) name = "Unconditional trap";
	if(code == 0x180) name = "Illegal instruction";
	if(code == 0x1a0) name = "Illegal delay slot";

	/* Custom gint codes for convenience */
	if(code == 0x1020) name = "DMA address error";
	if(code == 0x1040) name = "Add-in too large";
	if(code == 0x1060) name = "Memory init failed";

	if(name[0]) dtext(1, 9, name);
	else dprint(1, 9, "%03x", code);

	dprint(1, 17, " PC :%08x", PC);
	dprint(1, 25, "TEA :%08x", TEA);
	dprint(1, 33, "TRA :%08x", TRA);
	if(isSH4()) dprint(1, 41, "SGR :%08x", SGR);

	dtext(1, 49, "The add-in crashed.");
	dtext(1, 57, "Please reset the calc");
	#endif

	#ifdef FXCG50
	dtext(6, 3, "An exception occured! (System ERROR)");

	uint32_t *long_vram = (void *)gint_vram;
	for(int i = 0; i < 198 * 16; i++) long_vram[i] = ~long_vram[i];

	char const *name = "";
	if(code == 0x040) name = "TLB miss (nonexisting address) on read";
	if(code == 0x060) name = "TLB miss (nonexisting address) on write";
	if(code == 0x0e0) name = "Read address error (probably alignment)";
	if(code == 0x100) name = "Write address error (probably alignment)";
	if(code == 0x160) name = "Unconditional trap";
	if(code == 0x180) name = "Illegal instruction";
	if(code == 0x1a0) name = "Illegal delay slot instruction";

	/* Custom gint codes for convenience */
	if(code == 0x1020) name = "DMA address error";
	if(code == 0x1040) name = "Add-in not fully mapped (too large)";
	if(code == 0x1060) name = "Memory initialization failed (heap)";

	dprint(6, 25, "%03x %s", code, name);

	dtext(6, 45, "PC");
	dprint(38, 45, "= %08x", PC);
	dtext(261, 45, "(Error location)");

	dtext(6, 60, "TEA");
	dprint(38, 60, "= %08x", TEA);
	dtext(234, 60, "(Offending address)");

	dtext(6, 75, "TRA");
	dprint(38, 75, "= %#x", TRA);
	dtext(281, 75, "(Trap number)");

	dtext(6, 95,  "An unrecoverable error occurred in the add-in.");
	dtext(6, 108, "Please press the RESET button to restart the");
	dtext(6, 121, "calculator.");

	/* DMA address error handler */
	if(code == 0x1020)
	{
		#define DMA SH7305_DMA
		dprint(6, 141, "SAR0: %08x   DAR0: %08x   TCR0: %08x",
			DMA.DMA0.SAR, DMA.DMA0.DAR, DMA.DMA0.TCR);
		dprint(6, 154, "CHCR0: %08x", DMA.DMA0.CHCR);
		dprint(6, 167, "SAR1: %08x   DAR1: %08x   TCR1: %08x",
			DMA.DMA1.SAR, DMA.DMA1.DAR, DMA.DMA1.TCR);
		dprint(6, 180, "CHCR1: %08x", DMA.DMA1.CHCR);
		dprint(6, 193, "DMAOR: %04x", DMA.OR);
		#undef DMA
	}
	#endif

	dupdate();
	while(1) sleep();
}

/* Panic handler */
GNORETURN void (*gint_exc_panic)(uint32_t code) = gint_default_panic;
/* Exception catcher */
int (*gint_exc_catcher)(uint32_t code) = NULL;

/* gint_panic(): Panic handler function */
void gint_panic(uint32_t code)
{
	gint_exc_panic(code);
}

/* gint_panic_set(): Change the panic handler function */
void gint_panic_set(GNORETURN void (*panic)(uint32_t code))
{
	gint_exc_panic = panic ? panic : gint_default_panic;
}

/* gint_exc_catch(): Set a function to catch exceptions */
void gint_exc_catch(int (*handler)(uint32_t code))
{
	gint_exc_catcher = handler;
}

/* gint_exc_skip(): Skip pending exception instructions */
void gint_exc_skip(int instructions)
{
	uint32_t spc;

	/* Increase SPC by 2 bytes per instruction */
	__asm__("stc spc, %0" : "=r"(spc));
	spc += 2 * instructions;
	__asm__("ldc %0, spc" :: "r"(spc));
}
