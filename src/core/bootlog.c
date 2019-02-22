//---
//	gint:core:bootlog - Boot-time on-screen log for extreme debugging
//---

/* TODO: Review, enhance and fix bootlog */

#include <gint/defs/types.h>
#include <gint/mpu.h>
#include <gint/mpu/intc.h>
#include <core/mmu.h>
#include <gint/gint.h>
#include <gint/display.h>

#ifdef FXCG50
#define dclear(c) Bdisp_AllClr_VRAM()
#define dupdate() Bdisp_PutDisp_DD()
#endif

/* Linker script symbols - see core/start.c for details */
extern char
	brom, srom,
	sgdata, sgbss, sdata, sbss,
	btors, mtors, etors;

/* bootlog_loaded() - section loading stage */
void bootlog_loaded(void)
{
	/* Version string - the string constant resides in ROM */
	const char *base = "gint @";
	const char *hexa = "0123456789abcdef";
	char str[14];

	for(int i = 0; i < 6; i++) str[i] = base[i];
	str[13] = 0;

	/* Retrieve the commit number */
	for(int i = 0; i < 7; i++)
	{
		int shift = 24 - (i << 2);
		str[i + 6] = hexa[(GINT_VERSION >> shift) & 0xf];
	}

	/* Size of memory sections */
	uint32_t rom_size	= (uint32_t)&srom;
	uint32_t ram_size	= (uint32_t)&sdata + (uint32_t)&sbss;
	uint32_t gint_size	= (uint32_t)&sgdata + (uint32_t)&sgbss;

	/* MPU type */
	mpu_t mpu = gint_mpu();
	const char *names = "SH7337\0 SH7305\0 SH7355\0 SH7724";

	/* TODO: Use a solid API for boot-time printing */
	dclear(color_white);
	print(1, 1, str);
	print(15, 1, " Loaded");

	if((uint)mpu < 4) print(16, 2, names + 8 * (mpu - 1));
	else print_dec(16, 2, mpu, 6);

	print(1, 2, "ROM  RAM  GINT");
	print(4, 3, "k           c  d");
	print_dec(1, 3, (rom_size + 0x3ff) >> 10, 3);
	print_dec(6, 3, ram_size, 4);
	print_dec(11, 3, gint_size, 4);
	print_dec(17, 3, &mtors - &btors, 2);
	print_dec(20, 3, &etors - &mtors, 2);

	dupdate();
}

/* bootlog_mapped() - ROM mapping stage */
void bootlog_mapped(int rom, int ram)
{
	rom = (rom + 0x3ff) >> 10;
	ram = (ram + 0x3ff) >> 10;

	print(15, 1, " Mapped");
	print(1, 4, "MMU ROM:   k RAM:   k");
	(rom < 0) ? print(9,  4, "???") : print_dec(9,  4, rom, 3);
	(ram < 0) ? print(18, 4, "???") : print_dec(18, 4, ram, 3);

	dupdate();
}

/* bootlog_kernel() - gint loading stage */
void bootlog_kernel(void)
{
	print(15, 1, " Kernel");

	if(isSH3())
	{
		print(1, 5, "ABCD");
		print_hex( 6, 5, SH7705_INTC._.IPRA->word, 4);
		print_hex(10, 5, SH7705_INTC._.IPRB->word, 4);
		print_hex(14, 5, SH7705_INTC._.IPRC->word, 4);
		print_hex(18, 5, SH7705_INTC._.IPRD->word, 4);
		print(1, 6, "EFGH");
		print_hex( 6, 6, SH7705_INTC._.IPRE->word, 4);
		print_hex(10, 6, SH7705_INTC._.IPRF->word, 4);
		print_hex(14, 6, SH7705_INTC._.IPRG->word, 4);
		print_hex(18, 6, SH7705_INTC._.IPRH->word, 4);
	}
	else
	{
		print(1, 5, "ACFG");
		print_hex( 6, 5, SH7305_INTC._->IPRA.word, 4);
		print_hex(10, 5, SH7305_INTC._->IPRC.word, 4);
		print_hex(14, 5, SH7305_INTC._->IPRF.word, 4);
		print_hex(18, 5, SH7305_INTC._->IPRG.word, 4);
		print(1, 6, "HJKL");
		print_hex( 6, 6, SH7305_INTC._->IPRH.word, 4);
		print_hex(10, 6, SH7305_INTC._->IPRJ.word, 4);
		print_hex(14, 6, SH7305_INTC._->IPRK.word, 4);
		print_hex(18, 6, SH7305_INTC._->IPRL.word, 4);
	}

	dupdate();
}
