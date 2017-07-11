#include "gintdemo.h"

#include <mpu.h>
#include <gint.h>
#include <keyboard.h>
#include <display.h>
#include <gray.h>

#include <internals/stdio.h>
#include <stdio.h>



//---
//	A few procedures for displaying text aligned on a 21*8 grid.
//	Not really beautiful... but this will do.
//---

void locate(int x, int y, const char *str)
{
	if(x < 1 || x > 21 || y < 1 || y > 8) return;
	if(gray_runs()) gtext(x * 6 - 5, y * 8 - 8, str);
	else dtext(x * 6 - 5, y * 8 - 8, str);
}

void print(int x, int y, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	__printf(0, format, args);
	va_end(args);

	locate(x, y, __stdio_buffer);
}

/*
	printf_test()
	Tests formatting functions.

void printf_test(void)
{
	dclear();
	locate(1, 1, "Formatted printing");

	print(2, 3, "%%4.2d   5 :\"%4.2d\"", 5);
	print(2, 4, "%%-3c   '&':\"%-3c\"", '&');
	print(2, 5, "%%#05x  27 :\"%#05x\"", 27);
	print(2, 6, "%%1s   \"tr\":\"%1s\"", "tr");
	print(2, 7, "%%6p  NULL :\"%6p\"", NULL);

	dupdate();
	while(getkey() != KEY_EXIT);
}
*/

/*
	tlb_debug()
	Displays the TLB contents and some information. Only available for
	SH7705, because the SH7305's TLB is much more complicated.

void tlb_debug(void)
{
	// Entry address address (pointer in the address array), entry data
	// address (pointer in the data address), and their contents.
	unsigned int address, data, a, d;
	// Virtual page number and physical page number.
	unsigned int vpn, ppn;
	// Contents of register MMUCR.
	unsigned mmucr;

	int i, r, key = 0;
	int way = 0, entry = 0;
	int pointer_base;

	const char *protection[] = { "pr", "prw", "ar", "arw" };
	mmucr = *((volatile unsigned int *)gint_reg(Register_MMUCR));

	dclear();
	locate("MMU register info", 1, 1);
	locate("MMUCR.IX = ", 2, 3);
	locate(mmucr & 0x02 ? "1" : "0", 13, 3);
	dupdate();
	getkey();

	while(key != KEY_EXIT && way < 4)
	{
		dclear();

		print(1, 1, "TLB    way=%d   %d-%d", way, entry,
			entry > 29 ? 31 : entry + 2);

		for(i = 0; i < 3 && entry < 32; i++, entry++)
		{
			address	= 0xf2000000 | (entry << 12) | (way << 8);
			data	= 0xf3000000 | (entry << 12) | (way << 8);

			a = *((volatile unsigned int *)address);
			d = *((volatile unsigned int *)data);

			ppn = (d >> 10) & 0x00007ffff;
			// 4-kbyte page
			if(d & 0x08)
			{
				vpn = (a >> 12) | entry;
				pointer_base = vpn << 12;
			}
			// 1-kbyte page
			else
			{
				vpn = (a >> 10) | (entry << 2);
				pointer_base = vpn << 10;
			}

			r = 2 * i + 3;
			print(1, r, "%08x  :%08x", pointer_base, ppn << 10);

			r++;
			locate((d & 0x08) ? "4k" : "1k", 1, r);
			print(5, r, "pr=%s", protection[(d >> 5) & 3]);
			locate((d & 0x02) ? "shared" : "exclusive", 13, r);
		}

		if(entry == 32) entry = 0, way++;

		dupdate();
		key = getkey();
	}
}
*/

#include <internals/timer.h>

/*
	main_menu()
	Displays the main menu and returns user's choice: 0 for [EXIT],
	application numbers otherwise. Sets the ctaegory and application
	number.
*/
void main_menu(int *category, int *app)
{
	//---
	//	Quite a few things to declare...
	//---

	extern image_t res_opt_menu;

	const char *mpu, *mpu_names[] = {
		"Unknown",
		"SH7337",
		"SH7355",
		"SH7305",
		"SH7724",
		"[error]"
	};

	const char *list_tests[] = {
		"Keyboard & events",
		"Gray engine",
		"Image rendering",
		"Text rendering",
		"Real-time clock",
		"Clocks and timers",
		NULL
	};
/*
	const char *list_perfs[] = {
		"Image rendering",
		"Text rendering",
		NULL
	};
*/
	const char **list = NULL;
	int list_len = 0;

	extern unsigned int bgint, egint;
	extern unsigned int romdata;
	int gint_size = (char *)&egint - (char *)&bgint;

	// Building a version string.
	char gint_version[20];
	uint32_t v = (uint32_t)&GINT_VERSION;
	sprintf(gint_version, "%s-%d.%d-%d",
		(v >> 24 == 'a') ? "alpha" : (v >> 24 == 'b') ? "beta" :
		(v >> 24 == 'd') ? "dev" : (v >> 24 == 'r') ? "release" : "?",
		(v >> 20) & 0x0f, (v >> 16) & 0x0f, v & 0xffff);

	static int tab = 0, index = 0, scroll = 0;
	// Set to 1 when interface has to be redrawn.
	int leave = 1;
	int i;

	mpu = mpu_names[MPU_CURRENT < 5 ? MPU_CURRENT : 5];
	text_configure(NULL, color_black);

	while(1)
	{
		//---
		//	Displaying the current tab.
		//---

		dclear();

		switch(tab)
		{
		case 0:
			print(1, 1, "gint %s", gint_version);
			print(2, 3, "MPU type    %7s", mpu);
			print(2, 4, "Add-in size    %3dk",
				((uint32_t)&romdata - 0x00300000) >> 10);
			print(2, 5, "gint size    %5do", gint_size);

			list = NULL;
			break;

		case 1:
			locate(1, 1, "Test list");
			list = list_tests;
			break;
/*
		case 2:
			locate(1, 1, "Performance");
			list = list_perfs;
			break;
*/
		default:
			print(1, 1, "Tab %d", tab);
			break;
		}
		dimage_part(0, 56, &res_opt_menu, 0, 0, 42, 8);

		if(list)
		{
			list_len = 0;
			while(list[list_len]) list_len++;

			for(i = scroll; list[i] && i < scroll + 6; i++)
				locate(2, i - scroll + 2, list[i]);

			if(scroll > 0) locate(20, 2, "\x0d");
			if(scroll + 6 < list_len) locate(20, 7, "\x0e");

			drect(0, 8 * (index - scroll) + 8, 127,
				8 * (index - scroll) + 15, color_invert);
		}

		dupdate();

		//---
		//	Waiting for events.
		//---

		do
		{
			leave = 1;

			switch(getkey())
			{
			case KEY_F1:
				if(!tab) break;
				tab	= 0;
				index	= 0;
				break;
			case KEY_F2:
				if(tab == 1) break;
				tab	= 1;
				index	= 0;
				scroll	= 0;
				break;
/*			case KEY_F3:
				*category = 2;
				*app = 1;
				return;
*/
/*			case KEY_F3:
				if(tab == 2) break;
				tab	= 2;
				index	= 0;
				scroll	= 0;
				break;
*/
			case KEY_UP:
				if(list && list_len > 1)
				{
					if(index)
					{
						index--;
						if(index < scroll) scroll--;
					}
					else
					{
						index = list_len - 1;
						scroll = list_len - 6;
						if(scroll < 0) scroll = 0;
					}
				}
				else leave = 0;
				break;
			case KEY_DOWN:
				if(list && list_len > 1)
				{
					if(list[index + 1])
					{
						index++;
						if(index >= scroll + 6)
							scroll++;
					}
					else
					{
						index = 0;
						scroll = 0;
					}
				}
				else leave = 0;
				break;

			case KEY_EXE:
				if(!tab) break;
				if(category) *category = tab;
				if(app) *app = index + 1;
				return;

			default:
				leave = 0;
			}
		}
		while(!leave);
	}

	if(category) *category = 0;
	if(app) *app = 0;
	return;
}

/*
	main()
	No need for description.
*/
int main(void)
{
	int category, app;

	while(1)
	{
		main_menu(&category, &app);
		if(!category) break;

		switch((category << 8) | app)
		{
		case 0x0101: test_keyboard_events();		break;
		case 0x0102: test_gray();			break;
		case 0x0103: test_bopti();			break;
		case 0x0104: test_tales();			break;
		case 0x0105: test_rtc();			break;
		case 0x0106: test_timer();			break;

		case 0x0201: perf_bopti();			break;
		case 0x0202: /* perf_tales(); */		break;
		}
	}

	return 0;
}

/*
#include <bfile.h>
void screen(void)
{
	const uint8_t bmp_header[0x7e] = {
		0x42, 0x4d, 0x7e, 0x04, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x6c, 0x00,
		0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x40, 0x00,
		0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x13, 0x0b,
		0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x01, 0x00,
		0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x42, 0x47,
		0x52, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xff, 0xff, 0xff, 0x00,
	};
	const uint16_t file[] = { '\\', '\\', 'f', 'l', 's', '0', '\\', 'g',
		's', 'c', 'r', 'e', 'e', 'n', '.', 'b', 'm', 'p', 0x00 };
	int size = 0x7e + 1024;

	BFile_Remove(file);
	BFile_Create(file, BFile_File, &size);
	int handle = BFile_Open(file, BFile_WriteOnly);
	BFile_Write(handle, bmp_header, 0x7e);
	void *vram = (void *)display_getCurrentVRAM() + 1024;
	for(int i = 1; i <= 64; i++)
	{
		BFile_Write(handle, vram - 16 * i, 16);
	}
	BFile_Close(handle);
}
void screengray(void)
{
	const uint8_t bmp_header[0x7e] = {
		0x42, 0x4d, 0x7e, 0x04, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x6c, 0x00,
		0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x40, 0x00,
		0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x13, 0x0b,
		0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x01, 0x00,
		0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x42, 0x47,
		0x52, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xff, 0xff, 0xff, 0x00,
	};
	uint16_t file[] = { '\\', '\\', 'f', 'l', 's', '0', '\\', 'g', 's',
		'c', 'r', 'e', 'e', 'n', '#', '.', 'b', 'm', 'p', 0x00 };
	int size = 0x7e + 1024;
	void *vram;
	int handle;

	gupdate();

	file[14] = 'l';
	BFile_Remove(file);
	BFile_Create(file, BFile_File, &size);
	handle = BFile_Open(file, BFile_WriteOnly);
	BFile_Write(handle, bmp_header, 0x7e);
	vram = (void *)gray_lightVRAM() + 1024;
	for(int i = 1; i <= 64; i++)
	{
		BFile_Write(handle, vram - 16 * i, 16);
	}
	BFile_Close(handle);

	file[14] = 'd';
	BFile_Remove(file);
	BFile_Create(file, BFile_File, &size);
	handle = BFile_Open(file, BFile_WriteOnly);
	BFile_Write(handle, bmp_header, 0x7e);
	vram = (void *)gray_darkVRAM() + 1024;
	for(int i = 1; i <= 64; i++)
	{
		BFile_Write(handle, vram - 16 * i, 16);
	}
	BFile_Close(handle);

	gupdate();
}
*/
