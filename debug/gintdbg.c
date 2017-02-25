//---
//
//	gintdbg
//
//	A simple debugger for gint applications, providing diagnoses to
//	determine what went bad.
//
//---

// Just for structure definitions, gint does not run here.
#define	GINT_DIAGNOSTICS
#include <internals/gint.h>
#include <mpu.h>

#include <fxlib.h>
#include <stdarg.h>
#include <stddef.h>

// Some functions from other files...
int vsprintf(char *buffer, const char *format, va_list args);
int sprintf(char *buffer, const char *format, ...);

//---
//	Some util...
//---

static int print_row = 1;

void print(int col, const char *format, ...)
{
	int row = print_row;
	print_row++;
	if(row < 1 || row > 8) return;

	char buffer[256];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	locate(col, row);
	Print((unsigned char *)buffer);
}

void newline(void)
{
	print_row++;
}

void nothing_found(void)
{
	unsigned int key;

	Bdisp_AllClr_VRAM();
	PopUpWin(6);
	print_row = 2;
	print(3, "Apparently there");
	print(3, "is no diagnostic.");
	print(3, "");
	print(3, "Show anyway?");
	print(3, "   Yes:[F1]");
	print(3, "   No :[MENU]");

	do GetKey(&key);
	while(key != KEY_CTRL_F1);
}

void show_diagnostics(void)
{
	volatile gint_diagnostics_t *dg = gint_diagnostics();

	const char *stages[] = {
		"Startup", "Sections", "MMU", "Gint", "Clock", "Constructors",
		"Running", "Leaving", "Destructors", "Terminated",
	};
	const char *mpus[] = {
		"Unknown", "SH7337", "SH7355", "SH7305", "SH7724",
	};

	print(1, "Gint debugger (%d)", dg->counter);
	newline();

	print(1, "General information");
	print(2, "Magic    0x%02x", dg->magic);
	print(2, "Stage    %s", dg->stage <= 9 ? stages[dg->stage] : "-");
	if(dg->stage >= stage_gint)
	{
		print(2, "MPU      %s", dg->mpu <= 4 ? mpus[dg->mpu] : "-");
	}
	print(2, "Version  %08x", dg->version);
	newline();

	print(1, "Memory map");
	print(2, "%08x    romdata", dg->romdata);
	print(2, "%08x        vbr", dg->vbr_address);
	print(2, "%08x:%05x text", dg->section_text.address,
		dg->section_text.length);
	print(2, "%08x:%05x data", dg->section_data.address,
		dg->section_data.length);
	print(2, "%08x:%05x  bss", dg->section_bss.address,
		dg->section_bss.length);

	print(2, "%08x:%05x gint", dg->section_gint.address,
		dg->section_gint.length);
	newline();

	print(1, "Exception records");
	size_t len = sizeof dg->except_vect;
	char line[19];
	for(size_t i = 0; i < len; i += 6)
	{
		for(size_t n = 0; n < 6 && i + n < len; n++)
		{
			size_t index = (dg->excepts + i + n) % len;
			sprintf(line + 3 * n, " %02x", dg->except_vect[index]);
		}

		print(1, "%s", line);
	}
	print(2, "SPC     %08x", dg->spc);
	print(2, "SSR     %08x", dg->ssr);
	print(2, "EXPEVT  %08x", dg->expevt);
	print(2, "TEA     %08x", dg->tea);
	newline();

	if(dg->stage >= stage_clock)
	{
		print(1, "Clock frequencies");
		print(2, "Bus clock   %d MHz", dg->Bphi_f);
		print(2, "Peripheral  %d MHz", dg->Pphi_f);
		print(2, "Processor   %d MHz", dg->Iphi_f);
		newline();
	}
}

/*
	main()
	Let's do this!
*/
int main(void)
{
	volatile gint_diagnostics_t *dg = gint_diagnostics();
	unsigned int key;

	if(dg->magic != GINT_DIAGNOSTICS_MAGIC
		|| dg->stage > 9
		|| dg->mpu > 4
	) nothing_found();

	int total_height = -1;
	int y = 0;

	while(1)
	{
		Bdisp_AllClr_VRAM();
		print_row = 1 - y;

		show_diagnostics();
		if(total_height < 0) total_height = print_row - 1;

		// Drawing a scrollbar.
		if(total_height > 8)
		{
			int base = (64 * y) / total_height;
			int height = (64 * 8) / total_height;
			Bdisp_DrawLineVRAM(127, base, 127, base + height);
		}
		Bdisp_PutDisp_DD();

		do GetKey(&key);
		while(key != KEY_CTRL_UP && key != KEY_CTRL_DOWN);

		if(key == KEY_CTRL_UP && y > 0) y--;
		else if(key == KEY_CTRL_DOWN && y + 8 < total_height) y++;
	}

	return 1;
}
