//---
//	gint:core:bootlog - Boot-time on-screen log for extreme debugging
//---

#include <gint/defs/types.h>
#include <core/std.h>
#include <core/mmu.h>

#include <gint/mpu.h>
#include <gint/mpu/intc.h>

#include <gint/gint.h>
#include <gint/display.h>
#include <core/syscalls.h>
#include <gint/clock.h>

#ifdef FXCG50
void Bdisp_AllClr_VRAM(void);
void Bdisp_PutDisp_DD(void);

#define dclear(c) Bdisp_AllClr_VRAM()
#define dupdate() Bdisp_PutDisp_DD()
#endif

/* Linker script symbols - see core/start.c for details */
extern char
	brom, srom,
	sgdata, sgbss, sdata, sbss,
	btors, mtors, etors;

/* print() - formatted printing shorthand */
static void print(int x, int y, const char *format, ...)
{
	char str[45];
	va_list args;
	va_start(args, format);

	vsprintf(str + 2, format, args);

	#ifdef FX9860G
	dtext(6 * (x - 1) + 1, 7 * (y - 1), str + 2, color_black, color_white);
	#endif

	#ifdef FXCG50
	PrintXY(x, y, str, 0, 0);
	#endif

	va_end(args);
}

/* bootlog_loaded() - section loading stage */
GSECTION(".pretext")
void bootlog_loaded(void)
{
	/* Size of memory sections */
	uint32_t rom_size  = (uint32_t)&srom;
	uint32_t ram_size  = (uint32_t)&sdata + (uint32_t)&sbss;
	uint32_t gint_size = (uint32_t)&sgdata + (uint32_t)&sgbss;

	/* MPU type */
	mpu_t mpu = gint_mpu();
	const char *names = "SH7337\0 SH7305\0 SH7355\0 SH7724";

	/* TODO: Use a solid API for boot-time printing */
	dclear(color_white);
	print(1, 1, "gint @%7x   SLmkd", GINT_VERSION);

	if((uint)mpu < 4) print(1, 2, names + 8 * (mpu - 1));
	else print(1, 2, "%6d", mpu);

	#ifdef FX9860G
	print(8, 2, "%c?-", gint_platform.extended_ram ? 'R' : 'r');
	#endif

	#ifdef FXCG50
	print(8, 2, "--%c", gint_platform.prizm ? 'P' : 'p');
	#endif

	char os[11];
	__os_version(os);
	print(12, 2, "OS%2s%2s%4s", os, os+3, os+6);

	print(1, 3, "ROM%4dk RAM%3d+%1dk ??",
		(rom_size  + 0x3ff) >> 10,
		(ram_size  + 0x3ff) >> 10,
		(gint_size + 0x3ff) >> 10);

	dupdate();
}

/* bootlog_mapped() - ROM mapping stage */
GSECTION(".pretext")
void bootlog_mapped(int rom, int ram)
{
	/* Check whether all ROM is mapped */
	uint32_t rom_size  = (uint32_t)&srom;

	print(20, 3, "%c%c", (rom >= (int)rom_size) ? 'F' : 'f',
		isSH3() ? 'u' : 'U');
	print(19, 1, "M");

	#ifdef FX9860G
	print(9, 2, (ram > 8192) ? "E" : "e");
	#endif

	dupdate();
}

/* bootlog_unmapped() - ROM mapping stage failure */
void bootlog_unmapped(int rom, int size)
{
	print(1, 6, "MMU: add-in too large");
	print(1, 7, "%8x < %8x", rom, size);
	dupdate();
}

/* bootlog_kernel() - gint loading stage */
void bootlog_kernel(void)
{
	print(20, 1, "K");
	dupdate();
}

/* bootlog_driver() - driver load
   Called for very loaded driver. */
void bootlog_driver(const char *drv, const char *status)
{
	/* Positioning for the driver name */

	static int x = 1, y = 4;
	if(y > 5)
	{
		print(21, 4, "+");
		return;
	}

	print(x, y, "%3s", drv);
	x += 4;

	if(x + y >= 22) x=1, y++;

	/* Positioning for the driver message */

	if(!status) return;

	int len = strlen(status);

	static int mx = 1, my = 6;
	if(mx + len > 22) mx = 1, my++;
	if(my > 9) return;

	print(mx, my, "%s", status);
	mx += len + 1;

	dupdate();
}

/* bootlog_driver_summary() - all drivers loaded */
void bootlog_driver_summary(void)
{
	int interrupts = 0;
	uint16_t ipr;
	print(21, 1, "D");

	/* Count number of enabled interrupts */
	if(isSH3()) for(int i = 0; i < 8; i++)
	{
		ipr = *SH7705_INTC.IPRS[i];
		while(ipr > 0)
		{
			interrupts += (ipr & 0xf) != 0;
			ipr >>= 4;
		}
	}
	else for(int i = 0; i < 12; i++)
	{
		ipr = SH7305_INTC.IPRS[2 * i];
		while(ipr > 0)
		{
			interrupts += (ipr & 0xf) != 0;
			ipr >>= 4;
		}
	}

	print(19, 5, "#%2d", interrupts);

	dupdate();
}
