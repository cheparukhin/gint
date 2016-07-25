#include <stdlib.h>
#include <string.h>

#include <mpu.h>
#include <gint.h>
#include <keyboard.h>
#include <display.h>
#include <timer.h>
#include <gray.h>

#include <stdint.h>
#include <stdarg.h>
#include <7305.h>

#include <internals/stdio.h>



//---
//	A few procedures for displaying text aligned on a 21*8 grid.
//	Not really beautiful... but this will do.
//---

void print(int x, int y, const char *format, ...)
{
	if(x < 1 || x > 21 || y < 1 || y > 8) return;

	va_list args;
	va_start(args, format);
	__printf(0, format, args);
	va_end(args);

	if(gray_runs()) gtext(__stdio_buffer, x * 6 - 5, y * 8 - 8);
	else dtext(__stdio_buffer, x * 6 - 5, y * 8 - 8);
}

void locate(const char *str, int x, int y)
{
	if(x < 1 || x > 21 || y < 1 || y > 8) return;
	if(gray_runs()) gtext(str, x * 6 - 5, y * 8 - 8);
	else dtext(str, x * 6 - 5, y * 8 - 8);
}



//---
//	Test applications.
//---

/*
	keyboard_test_binary()
	Prints a byte as binary/
*/
void keyboard_test_binary(int byte, int x, int y)
{
	char str[9];
	int i = 7;

	str[8] = 0;
	while(i >= 0)
	{
		str[i] = '0' + (byte & 1);
		byte >>= 1;
		i--;
	}

	locate(str, x, y);
}

/*
	keyboard_test_timer()
	Displays a keyboard test. The keyboard state is displayed as well, but	
	there is a timer artifact. The keyboard state timer uses the same
	period as the (internal) keyboard analysis timer, but there is a time
	gap between a keyboard analysis and an update on the screen.
*/
void keyboard_test_timer(void)
{
	volatile unsigned char *state = keystate();

	dclear_area(5, 10, 71, 34);

	keyboard_test_binary(state[0], 1, 1);
	keyboard_test_binary(state[1], 1, 2);
	keyboard_test_binary(state[2], 1, 3);
	keyboard_test_binary(state[3], 1, 4);
	keyboard_test_binary(state[4], 1, 5);

	keyboard_test_binary(state[5], 10, 1);
	keyboard_test_binary(state[6], 10, 2);
	keyboard_test_binary(state[7], 10, 3);
	keyboard_test_binary(state[8], 10, 4);
	keyboard_test_binary(state[9], 10, 5);

	dupdate();
}

/*
	keyboard_test()
	Displays a multi-getkey test as well as the keyboard state in real
	time.
*/
void keyboard_test(void)
{
	int x = 0;
	char str[3];
	int keys[4] = { 0 };
	int i;

	timer_start(TIMER_USER, 1700, TIMER_Po_256, keyboard_test_timer, 0);

	dclear();
	locate("Keyboard state:", 0, 0);
	locate("multi-getkey ^^", 50, 55);
	dupdate();

	while(1)
	{
		multigetkey(keys, 4, 0);
		if(keys[0] == KEY_EXIT && keys[1] == KEY_NONE) break;


		#define	hexa(h) ('0' + (h) + 39 * ((h) > 9))

		x = (x + 1) & 15;
		str[0] = hexa(x);
		str[1] = 0;
		locate(str, 100, 0);

		for(i = 0; i < 4; i++)
		{
			str[0] = hexa((keys[i] >> 4) & 0x0f);
			str[1] = hexa(keys[i] & 0x0f);
			str[2] = 0;
			locate(str, 100, 16 + 10 * i);
		}

		#undef	hexa

		dupdate();
	}

	timer_stop(TIMER_USER);
}


/*
	bitmap_test()
	Displays various bitmaps to ensure bopti is working correctly.
*/
/*
void bitmap_test(void)
{
	extern Image res_bitmap_opt_start;
	extern Image res_symbol_start;
	extern Image res_symbol2_start;
	extern Image res_sprites_start;
	extern Image res_swords_start;
	extern Image res_items_start;

	Image *opt	= &res_bitmap_opt_start;
	Image *sybl	= &res_symbol_start;
	Image *sybl2	= &res_symbol2_start;
	Image *sprites	= &res_sprites_start;
	Image *swords	= &res_swords_start;
	Image *items	= &res_items_start;

	uint32_t a32 = 0xffffffff;
	int black_bg = 0, gray = 0;
	int key;
	int x = 20, y = 10;

	while(1)
	{
		if(gray)
		{
			gray_start();
			gclear();

			if(black_bg) greverse_area(0, 0, 127, 63);
			gimage(swords, 0, 57);

			gimage(items, x, y);

			gupdate();
		}
		else
		{
			gray_stop();
			dclear();

			if(black_bg) dreverse_area(0, 0, 127, 63);
			dimage(opt, 0, 57);

			dimage(sybl, 30 & a32, 40);
			dimage(sybl2, 62 & a32, 40);

			dupdate();

		}

		key = getkey();
		if(key == KEY_EXIT) break;

		if(key == KEY_F1) gray = !gray;
		if(key == KEY_F2) a32 ^= 31;
		if(key == KEY_F3) black_bg = !black_bg;

		if(key == KEY_UP) y--;
		if(key == KEY_DOWN) y++;
		if(key == KEY_LEFT) x--;
		if(key == KEY_RIGHT) x++;
	}

	gray_stop();
	return;
}
*/


/*
	text_test()
	Renders text.
*/
void text_test(void)
{
	extern Font res_font_modern_start;
	Font *font = &res_font_modern_start;

	text_configure(font);

	dclear();

	dtext(" !\"#$%&'()*+,-./", 10, 10);
	dtext("0123456789:;<=>?", 10, 16);
	dtext("@ABCDEFGHIJKLMNO", 10, 22);
	dtext("PQRSTUVWXYZ[\\]^_", 10, 28);
	dtext("`abcdefghijklmno", 10, 34);
	dtext("pqrstuvwxyz{|}~", 10, 40);

	dupdate();

	while(getkey() != KEY_EXIT);
}



/*
	gray_test()
	Runs the gray engine.
*/
void gray_test(void)
{
	extern Image res_opt_gray_start;

	int *v1, *v2;

	int delays[2]; // { light, dark }
	int key, changed = 1, i;
	int selected = 0;

	gray_getDelays(delays, delays + 1);
	gray_start();

	while(1)
	{
		if(changed)
		{
			gray_setDelays(delays[0], delays[1]);
			gclear();

			v1 = gray_lightVRAM();
			v2 = gray_darkVRAM();

			for(i = 0; i < 63; i++)
			{
				v1[(i << 2)] = v1[(i << 2) + 1] =
					-((i & 31) < 16);
				v2[(i << 2)] = v2[(i << 2) + 1] =
					-(i < 32);
			}

			locate("light", 15, 2);
			print(15, 3, "%d", delays[0]);

			locate("dark", 15, 5);
			print(15, 6, "%d", delays[1]);

			locate("\x02", 14, selected ? 6 : 3);

			gimage(&res_opt_gray_start, 0, 56);
			gupdate();
		}

		changed = 0;

		key = getkey_opt(Getkey_RepeatArrowKeys, 1);
		if(key == KEY_EXIT) break;

		changed = 1;

		switch(key)
		{
		case KEY_F1:
			selected = !selected;
			break;
		case KEY_F2:
			delays[0] = isSH3() ? 985 : 994;
			delays[1] = 1609;
			break;
		case KEY_F3:
			delays[0] = 860;
			delays[1] = 1298;
			break;
		case KEY_UP:
			delays[selected] += 10;
			break;
		case KEY_DOWN:
			if(delays[selected] >= 110) delays[selected] -= 10;
			break;
		case KEY_RIGHT:
			delays[selected]++;
			break;
		case KEY_LEFT:
			if(delays[selected] >= 101) delays[selected]--;
			break;
		default:
			changed = 0;
			break;
		}
	}

	gray_stop();
}

/*
	printf_test()
	Tests formatting functions.
*/
void printf_test(void)
{
	dclear();
	locate("Formatted printing", 1, 1);

	print(2, 3, "%%4.2d   5 :\"%4.2d\"", 5);
	print(2, 4, "%%-3c   '&':\"%-3c\"", '&');
	print(2, 5, "%%#05x  27 :\"%#05x\"", 27);
	print(2, 6, "%%1s   \"tr\":\"%1s\"", "tr");
	print(2, 7, "%%6p  NULL :\"%7p\"", NULL);

	dupdate();
	while(getkey() != KEY_EXIT);
}

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

	timer_start(TIMER_USER, 0x0fffffff, TIMER_Po_4, NULL, 0);
	for(i = 0; i < 1000; i++) ML_bmp_or_cl(screen, 1, 1, 128, 64);
	time1 = timer->TCNT;
	timer_stop(TIMER_USER);
	time1 = 0x0fffffff - time1;

	dclear();
	dtext("gint...", 2, 2);
	dupdate();

	timer_start(TIMER_USER, 0x0fffffff, TIMER_Po_4, NULL, 0);
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
*/
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

	extern Image res_opt_menu_start;

	const char *mpu, *mpu_names[] = {
		"Unknown",
		"SH7337",
		"SH7355",
		"SH7305",
		"SH7724",
		"[error]"
	};

	const char *list_tests[] = {
		"Keyboard",
		"Gray engine",
		"Image rendering",
		"Text rendering",
		"Real-time clock",
		"Text formatting",
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
	const char **list;
	int list_len;

	extern unsigned int bgint, egint;
	extern unsigned int romdata;
	int gint_size = (char *)&egint - (char *)&bgint;



	static int tab = 0, index = 0, scroll = 0;
	// Set to 1 when interface has to be redrawn.
	int leave = 1;
	int i;

	mpu = mpu_names[MPU_CURRENT < 5 ? MPU_CURRENT : 5];
	text_configure_default();

	while(1)
	{
		//---
		//	Displaying the current tab.
		//---

		dclear();
		dupdate();

		switch(tab)
		{
		case 0:
			locate("Demo application", 1, 1);
			print(2, 3, "gint version: %5s", GINT_VERSION_STR);
			print(2, 4, "handler size: %5d", gint_size);
			print(2, 5, "mpu type:   %7s", mpu);
			print(2, 6, "romdata:   %08x", &romdata);

			list = NULL;
			break;

		case 1:
			locate("Test list", 1, 1);
			list = list_tests;
			break;

		case 2:
			locate("Performance", 1, 1);
			list = list_perfs;
			break;

		case 3:
			locate("Debug", 1, 1);
			list = list_debug;
			break;

		default:
			print(1, 1, "Tab %d", tab);
			break;
		}
		dimage(&res_opt_menu_start, 0, 56);

		if(list)
		{
			list_len = 0;
			while(list[list_len]) list_len++;

			for(i = scroll; list[i] && i < scroll + 6; i++)
				locate(list[i], 2, i - scroll + 2);

			if(scroll > 0) locate("\x0d", 20, 2);
			if(scroll + 6 < list_len) locate("\x0e", 20, 7);

			dreverse_area(0, 8 * (index - scroll) + 8, 127,
				8 * (index - scroll) + 15);
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
				tab	= 0;
				index	= 0;
				break;
			case KEY_F2:
				tab	= 1;
				index	= 0;
				scroll	= 0;
				break;
			case KEY_F3:
				tab	= 2;
				index	= 0;
				scroll	= 0;
				break;
			case KEY_F4:
				tab	= 3;
				index	= 0;
				scroll	= 0;
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
		case 0x0101:
			keyboard_test();
			break;
		case 0x0102:
			gray_test();
			break;
		case 0x0103:
//			bitmap_test();
			break;
		case 0x0104:
			text_test();
			break;
		case 0x0105:
//			rtc_test();
			break;
		case 0x0106:
			printf_test();
			break;

		case 0x0301:
			if(isSH3()) tlb_debug();
			break;
		}
	}

	return 0;
}
