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
static const unsigned char screen[1024] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 7, 159, 0, 0, 1, 192, 0, 0, 0, 0, 0, 121, 240, 0, 0, 0,
31, 191, 192, 0, 3, 224, 27, 216, 0, 0, 1, 251, 252, 0, 0, 0, 57, 247, 222,
30, 7, 240, 36, 36, 62, 25, 131, 159, 24, 255, 129, 224, 0, 227, 142, 126, 1,
192, 45, 172, 127, 127, 192, 14, 1, 255, 199, 224, 0, 227, 140, 240, 1, 192,
26, 88, 115, 127, 224, 14, 57, 221, 207, 0, 0, 227, 13, 192, 1, 192, 34, 68,
120, 30, 0, 14, 25, 156, 220, 0, 0, 227, 253, 252, 1, 192, 36, 36, 126, 28,
0, 14, 219, 156, 223, 192, 0, 227, 253, 252, 1, 192, 36, 36, 31, 12, 0, 46,
27, 140, 223, 192, 0, 227, 141, 193, 193, 192, 40, 20, 7, 140, 0, 206, 25, 140,
220, 28, 0, 227, 140, 225, 129, 199, 24, 24, 99, 156, 1, 14, 25, 204, 206, 24,
0, 227, 142, 127, 1, 195, 39, 228, 255, 156, 2, 14, 24, 237, 199, 240, 1, 247,
222, 62, 1, 198, 44, 44, 223, 30, 2, 31, 28, 237, 131, 224, 1, 224, 0, 0, 3,
254, 27, 216, 0, 0, 4, 30, 0, 0, 0, 0, 3, 192, 0, 0, 7, 252, 0, 0, 0, 0, 4,
60, 1, 249, 240, 0, 0, 0, 0, 0, 0, 56, 0, 0, 0, 0, 4, 0, 97, 240, 56, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 1, 224, 28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
4, 0, 47, 192, 30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 32, 255, 128, 63, 128,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 32, 255, 0, 48, 78, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 15, 176, 255, 0, 112, 49, 0, 0, 0, 0, 0, 0, 0, 0, 0, 28, 8, 56, 255, 0,
96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 98, 8, 60, 255, 0, 224, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 130, 56, 126, 255, 3, 224, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 192,
62, 255, 15, 224, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 14, 191, 255, 192, 0,
0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 6, 129, 255, 192, 0, 0, 0, 0, 0, 0, 0, 0, 0,
1, 0, 0, 6, 0, 255, 192, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 7, 128, 63, 192,
0, 0, 96, 1, 224, 1, 0, 0, 0, 2, 0, 0, 7, 0, 31, 192, 0, 0, 95, 1, 11, 68, 88,
0, 0, 4, 0, 0, 7, 128, 31, 192, 0, 1, 192, 129, 204, 85, 100, 0, 0, 8, 0, 0,
15, 128, 63, 224, 0, 0, 95, 1, 8, 85, 68, 0, 1, 144, 0, 0, 31, 128, 143, 224,
64, 0, 96, 1, 232, 41, 68, 0, 2, 96, 0, 31, 255, 129, 7, 248, 96, 0, 0, 0, 0,
0, 0, 0, 4, 0, 0, 96, 254, 129, 7, 254, 96, 0, 0, 0, 0, 0, 0, 0, 4, 0, 1, 128,
254, 131, 135, 255, 224, 0, 0, 1, 192, 64, 16, 0, 8, 0, 7, 0, 254, 131, 255,
63, 224, 0, 0, 1, 38, 113, 208, 0, 8, 0, 13, 0, 222, 147, 254, 31, 224, 0, 0,
1, 41, 74, 80, 0, 8, 0, 25, 0, 222, 67, 254, 31, 160, 0, 0, 1, 41, 74, 80, 0,
12, 0, 49, 0, 222, 19, 254, 62, 48, 0, 0, 1, 198, 113, 208, 0, 2, 0, 32, 128,
222, 195, 255, 252, 56, 0, 0, 0, 0, 0, 0, 0, 2, 0, 124, 64, 220, 151, 135, 248,
127, 0, 0, 0, 0, 0, 0, 0, 2, 0, 66, 32, 221, 223, 7, 240, 255, 0, 0, 0, 0, 0,
0, 0, 2, 0, 129, 23, 93, 159, 15, 241, 131, 0, 0, 0, 0, 0, 0, 0, 4, 0, 128,
136, 217, 95, 3, 226, 9, 0, 0, 1, 240, 0, 0, 0, 4, 0, 128, 72, 89, 95, 129,
228, 18, 0, 0, 0, 0, 0, 0, 0, 4, 1, 0, 72, 73, 127, 128, 224, 36, 0, 0, 0, 0,
0, 0, 0, 28, 1, 0, 76, 129, 127, 192, 96, 8, 0, 0, 0, 0, 0, 0, 0, 16, 1, 0,
231, 203, 124, 96, 64, 0, 0, 0, 0, 0, 0, 0, 0, 16, 1, 1, 28, 123, 240, 12, 64,
1, 0, 0, 0, 0, 0, 0, 0, 16, 1, 2, 28, 143, 128, 15, 192, 7, 0, 0, 0, 0, 0, 0,
0, 16, 1, 4, 17, 143, 24, 15, 192, 14, 0, 0, 0, 0, 0, 0, 0, 28, 1, 4, 1, 135,
24, 31, 192, 24, 0, 0, 0, 0, 0, 0, 0, 18, 1, 62, 1, 135, 248, 63, 224, 192,
0, 0, 0, 0, 0, 0, 0, 35, 1, 195, 1, 135, 128, 254, 126, 1, 0, 0, 0, 0, 0, 0,
0, 35, 193, 131, 195, 135, 255, 248, 112, 1, 0, 0, 0, 0, 0, 0, 0, 67, 241, 131,
14, 207, 255, 192, 224, 3, 0, 0, 0, 0, 0, 0, 3, 67, 15, 143, 56, 255, 7, 1,
224, 7, 0, 0, 0, 0, 0, 0, 28, 130, 7, 255, 112, 204, 7, 131, 224, 31, 0, 0,
0, 0, 0, 0, 32, 134, 30, 29, 120, 156, 7, 255, 224, 127, 0, 0, 0, 0, 0, 63,
197, 206, 60, 56, 192, 248, 15, 255, 248, 255, 0, 0, 0, 0, 0, 120, 5, 227, 248,
56, 195, 248, 127, 191, 254, 63, 0, 0, 0, 0, 7, 254, 255, 193, 255, 15, 193,
255, 15, 31, 252, 31 };

void ML_bmp_or_cl(const unsigned char *bmp, int x, int y, int width, int height)
{
	unsigned short line;
	char shift, *screen, *p;
	int i, j, real_width, begin_x, end_x, begin_y, end_y;
	char bool1=1, bool2=1, bool3;
	if(!bmp || x<1-width || x>127 || y<1-height || y>63 || height<1 || width<1) return;
	p = (char*)&line;
	real_width = (width-1>>3<<3)+8;
	if(y < 0) begin_y = -y;
	else begin_y = 0;
	if(y+height > 64) end_y = 64-y;
	else end_y = height;
	shift = 8-(x&7);
	if(x<0)
	{
		begin_x = -x>>3;
		if(shift != 8) bool1 = 0;
	} else begin_x = 0;
	if(x+real_width > 128) end_x = 15-(x>>3), bool2 = 0;
	else end_x = real_width-1>>3;
	bool3 = (end_x == real_width-1>>3);
	screen = display_getCurrentVRAM()+(y+begin_y<<4)+(x>>3);

	for(i=begin_y ; i<end_y ; i++)
	{
		if(begin_x < end_x)
		{
			line = bmp[i*(real_width>>3)+begin_x] << shift;
			if(bool1) screen[begin_x] |= *p;
			if(shift!=8) screen[begin_x+1] |= *(p+1);
			for(j=begin_x+1 ; j<end_x ; j++)
			{
				line = bmp[i*(real_width>>3)+j] << shift;
				screen[j] |= *p;
				if(shift!=8) screen[j+1] |= *(p+1);
			}
		}
		line = bmp[i*(real_width>>3)+end_x];
		if(bool3) line &= -1<<real_width-width;
		line <<= shift;
		if(begin_x < end_x || bool1) screen[end_x] |= *p;
		if(bool2) screen[end_x+1] |= *(p+1);
		screen += 16;
	}
}

#include <internals/timer.h>
void debug(void)
{
	extern Image res_screen_start;
	struct mod_tmu *timer;
	int time1, time2;
	int i;

	timer_get(TIMER_USER, &timer, NULL);

	dclear();
	ML_bmp_or_cl(screen, 1, 1, 128, 64);
	dupdate();
	getkey();

	dclear();
	dimage(&res_screen_start, 1, 1);
	dupdate();
	getkey();

	dclear();
	dtext("ML...", 2, 2);
	dupdate();

	timer_start2(TIMER_USER, 0x0fffffff, TIMER_Po_4, NULL, 0);
	for(i = 0; i < 1000; i++) ML_bmp_or_cl(screen, 1, 1, 128, 64);
	time1 = timer->TCNT;
	timer_stop(TIMER_USER);
	time1 = 0x0fffffff - time1;

	dclear();
	dtext("gint...", 2, 2);
	dupdate();

	timer_start2(TIMER_USER, 0x0fffffff, TIMER_Po_4, NULL, 0);
	for(i = 0; i < 1000; i++) dimage(&res_screen_start, 1, 1);
	time2 = timer->TCNT;
	timer_stop(TIMER_USER);
	time2 = 0x0fffffff - time2;

	dclear();
	print_hex(time1, 2, 2);
	print_hex(time2, 2, 9);
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
		"Keyboard and events",
		"Gray engine",
		"Image rendering",
		"Text rendering",
		"Real-time clock",
		"Clocks and timers",
		NULL
	};
	const char *list_perfs[] = {
		"Image rendering",
		"Text rendering",
		NULL
	};
	const char *list_debug[] = {
		"View TLB (SH3 only)",
		NULL
	};
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
			print(2, 4, "handler size: %5d", gint_size);
			print(2, 5, "mpu type:   %7s", mpu);
			print(2, 6, "romdata:   %08x", &romdata);

			list = NULL;
			break;

		case 1:
			locate(1, 1, "Test list");
			list = list_tests;
			break;

		case 2:
			locate(1, 1, "Performance");
			list = list_perfs;
			break;

		case 3:
			locate(1, 1, "Debug");
			list = list_debug;
			break;

		default:
			print(1, 1, "Tab %d", tab);
			break;
		}
		dimage(0, 56, &res_opt_menu);

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
//			case KEY_7:
//				crt0_system_menu();
//				break;
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
			case KEY_F3:
				if(tab == 2) break;
				tab	= 2;
				index	= 0;
				scroll	= 0;
				break;
			case KEY_F4:
				if(tab == 3) break;
				tab	= 3;
				index	= 0;
				scroll	= 0;
				break;
			case KEY_F6:;
				void screen(void);
				screen();
				break;

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

			case KEY_MENU:
				if(category) *category = 0;
				if(app) *app = 0;
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

		case 0x0201: /* perf_bopti(); */		break;
		case 0x0202: /* perf_tales(); */		break;

		case 0x0301: /* if(isSH3()) debug_tlb(); */	break;
		}
	}

	return 0;
}

void crash(void)
{
	__asm__(
		"mov	#0, r0		\n\t"
		"ldc	r0, vbr		\n\t"
		"trapa	#1		"
	);
}

void screen(void)
{
	enum { File = 1, Folder = 5 };
	enum { Read = 0x01, Write = 0x02, ReadWrite = Read | Write };

	int BFile_Remove(const uint16_t *file);
	int BFile_Create(const uint16_t *file, int type, int *size);
	int BFile_Open(const uint16_t *file, int mode);
	int BFile_Close(int handle);
	int BFile_Write(int handle, const void *ram_buffer, int even_size);

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
	BFile_Create(file, File, &size);
	int handle = BFile_Open(file, Write);
	BFile_Write(handle, bmp_header, 0x7e);
	void *vram = (void *)display_getCurrentVRAM() + 1024;
	for(int i = 1; i <= 64; i++)
	{
		BFile_Write(handle, vram - 16 * i, 16);
	}
	BFile_Close(handle);
}
