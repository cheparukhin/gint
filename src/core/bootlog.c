//---
//	gint:core:bootlog - Boot-time on-screen log for extreme debugging
//---

#include <defs/types.h>
#include <core/intc.h>
#include <core/mpu.h>
#include <core/mmu.h>
#include <gint/gint.h>

/* Linker script symbols - see core/start.c for details */
extern char
	brom, srom,
	sgdata, sgbss, sdata, sbss,
	btors, mtors, etors;

/* bootlog_loaded() - section loading stage */
void bootlog_loaded(void)
{
	/* Version string - the string constant resides in ROM */
	uint32_t v = gint_version();
	const char *model = "gint #0.0-000";
	char str[14];

	for(int i = 0; i < 14; i++) str[i] = model[i];

	/* Quickly get the three digits of the build number */
	int x_q = (v & 0xffff) / 10;
	int x_r = (v & 0xffff) - 10 * x_q;
	int y_q = x_q / 10;
	int y_r = x_q - 10 * y_q;

	str[5]   = (v & 0xff000000) >> 24;
	str[6]  += (v & 0x00f00000) >> 20;
	str[8]  += (v & 0x000f0000) >> 16;
	str[10] += y_q;
	str[11] += y_r;
	str[12] += x_r;

	/* Size of memory sections */
	uint32_t rom_size	= (uint32_t)&srom;
	uint32_t ram_size	= (uint32_t)&sdata + (uint32_t)&sbss;
	uint32_t gint_size	= (uint32_t)&sgdata + (uint32_t)&sgbss;

	/* MPU type */
	mpu_t mpu = mpu_id();
	const char *names = "SH7337\0 SH7305\0 SH7355\0 SH7724";

	/* TODO: Use a solid API for boot-time printing */
	Bdisp_AllClr_VRAM();
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

	Bdisp_PutDisp_DD();
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

	Bdisp_PutDisp_DD();
}

/* bootlog_kernel() - gint loading stage */
void bootlog_kernel(void)
{
	print(15, 1, " Kernel");

	mpuSwitch({
		/* SH3-based */
		print(1, 5, "ABCD");
		print_hex( 6, 5, INTC3._.IPRA->word, 4);
		print_hex(10, 5, INTC3._.IPRB->word, 4);
		print_hex(14, 5, INTC3._.IPRC->word, 4);
		print_hex(18, 5, INTC3._.IPRD->word, 4);
		print(1, 6, "EFGH");
		print_hex( 6, 6, INTC3._.IPRE->word, 4);
		print_hex(10, 6, INTC3._.IPRF->word, 4);
		print_hex(14, 6, INTC3._.IPRG->word, 4);
		print_hex(18, 6, INTC3._.IPRH->word, 4);
	},{
		/* SH4-based */
		print(1, 5, "ACFG");
		print_hex( 6, 5, INTC4._->IPRA.word, 4);
		print_hex(10, 5, INTC4._->IPRC.word, 4);
		print_hex(14, 5, INTC4._->IPRF.word, 4);
		print_hex(18, 5, INTC4._->IPRG.word, 4);
		print(1, 6, "HJKL");
		print_hex( 6, 6, INTC4._->IPRH.word, 4);
		print_hex(10, 6, INTC4._->IPRJ.word, 4);
		print_hex(14, 6, INTC4._->IPRK.word, 4);
		print_hex(18, 6, INTC4._->IPRL.word, 4);
	});

	Bdisp_PutDisp_DD();
}
