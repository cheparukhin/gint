#define GINT_NEED_VRAM
#include <gint/display.h>
#include <gint/exc.h>
#include <gint/defs/attributes.h>

#define dprint(x, y, ...) dprint(x, y, C_BLACK, C_NONE, __VA_ARGS__)
#define dtext(x, y, str)  dtext (x, y, str, C_BLACK, C_NONE)

/* gint_exc(): Exception handler fatal error message */
void gint_exc(GUNUSED uint32_t code)
{
	uint32_t TEA = *(volatile uint32_t *)0xff00000c;
	uint32_t TRA = *(volatile uint32_t *)0xff000020 >> 2;

	uint32_t PC;
	__asm__("stc spc, %0" : "=r"(PC));

	dfont(NULL);
	dclear(C_WHITE);

	#ifdef FX9860G
	dtext(1, 0, "Exception! (SysERROR)");
	for(int i = 0; i < 32; i++) vram[i] = ~vram[i];

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

	if(name[0]) dtext(1, 9, name);
	else dprint(1, 9, "%03x", code);

	dprint(1, 17, " PC :%08x", PC);
	dprint(1, 25, "TEA :%08x", TEA);
	dprint(1, 33, "TRA :%08x", TRA);

	dtext(1, 49, "The add-in crashed.");
	dtext(1, 57, "Please reset the calc");
	#endif

	#ifdef FXCG50
	dtext(6, 3, "An exception occured! (System ERROR)");

	uint32_t *long_vram = (void *)vram;
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

	dtext(6, 95,  "An unrecoverable error ocurred in the add-in.");
	dtext(6, 108, "Please press the RESET button to restart the");
	dtext(6, 121, "calculator.");
	#endif

	dupdate_noint();
}

/* gint_exch_tlbh(): Exception and TLB miss handler */
__attribute__((interrupt_handler)) GSECTION(".gint.exch_tlbh") GALIGNED(4)
void gint_exch_tlbh(void)
{
	uint32_t EXPEVT = *(volatile uint32_t *)0xff000024;

	gint_exc(EXPEVT);
	while(1);
}
