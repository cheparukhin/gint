#include <stdlib.h>
#include <string.h>

#include <mpu.h>
#include <keyboard.h>
#include <display.h>
#include <timer.h>
#include <gray.h>

#include <stdint.h>
#include <7305.h>

//---
//	A few ugly procedures for displaying text. Will have to enhance this
//	soon -- which means printf().
//---

void print(const char *str, int x, int y)
{
	dtext(str, x, y);
}
void print_hex(unsigned int n, int x, int y)
{
	char ch[11] = "0x";
	int i;

	for(i = 0; i < 8; i++)
	{
		ch[9 - i] = (n & 0xf) + '0' + 39 * ((n & 0xf) > 9);
		n >>= 4;
	}
	ch[10] = 0;
	print(ch, x, y);
}
void print_bin(unsigned char n, int x, int y)
{
	char ch[9];
	int i;

	for(i = 0; i < 8;i ++)
	{
		ch[7 - i] = (n & 1) + '0';
		n >>= 1;
	}
	ch[8] = 0;
	print(ch, x, y);
}
void print_hexa(unsigned int n, int digits, int x, int y)
{
	char ch[20];
	int i;

	for(i = digits - 1; i >= 0; i--)
	{
		ch[i] = (n & 0xf) + '0' + 39 * ((n & 0xf) > 9);
		n >>= 4;
	}

	ch[digits] = 0;
	print(ch, x, y);
}
void print_int(int n, int x, int y)
{
	char str[20] = { 0 };
	int i, o = 0;
	int digits = 0, copy = n;

	if(!n) { str[o++] = '0'; print(str, x, y); return; }
	if(n < 0) str[o++] = '-', n = -n;
	while(copy) digits++, copy /= 10;

	for(i = 0; i < digits; i++)
	{
		str[o + digits - i - 1] = n % 10 + '0';
		n /= 10;
	}

	gtext(str, x, y);
}



//---
//	Test applications.
//---

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

	print_bin(state[0], 5, 10);
	print_bin(state[1], 5, 16);
	print_bin(state[2], 5, 22);
	print_bin(state[3], 5, 28);
	print_bin(state[4], 5, 34);

	print_bin(state[5], 40, 10);
	print_bin(state[6], 40, 16);
	print_bin(state[7], 40, 22);
	print_bin(state[8], 40, 28);
	print_bin(state[9], 40, 34);

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
	print("Keyboard state:", 0, 0);
	print("multi-getkey ^^", 50, 55);
	dupdate();

	while(1)
	{
		multigetkey(keys, 4, 0);
		if(keys[0] == KEY_EXIT && keys[1] == KEY_NONE) break;


		#define	hexa(h) ('0' + (h) + 39 * ((h) > 9))

		x = (x + 1) & 15;
		str[0] = hexa(x);
		str[1] = 0;
		print(str, 100, 0);

		for(i = 0; i < 4; i++)
		{
			str[0] = hexa((keys[i] >> 4) & 0x0f);
			str[1] = hexa(keys[i] & 0x0f);
			str[2] = 0;
			print(str, 100, 16 + 10 * i);
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
void bitmap_test(void)
{
	extern Image res_bitmap_opt_start;
	extern Image res_symbol_start;
	extern Image res_symbol2_start;
	extern Image res_sprites_start;
	extern Image res_swords_start;

	Image *opt	= &res_bitmap_opt_start;
	Image *sybl	= &res_symbol_start;
	Image *sybl2	= &res_symbol2_start;
	Image *sprites	= &res_sprites_start;
	Image *swords	= &res_swords_start;

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
			gimage(opt, 0, 57);

			gimage(sprites, x, y);
			gimage(swords, x, y + 35);

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



/*
	text_test()
	Renders text.
*/

void text_test(void)
{
	extern Font res_font_start;
	Font *font = &res_font_start;

	text_configure(font);

	dclear();

	print(" !\"#$%&'()*+,-./", 10, 10);
	print("0123456789:;<=>?", 10, 16);
	print("@ABCDEFGHIJKLMNO", 10, 22);
	print("PQRSTUVWXYZ[\\]^_", 10, 28);
	print("`abcdefghijklmno", 10, 34);
	print("pqrstuvwxyz{|}~", 10, 40);

	dupdate();

	while(getkey() != KEY_EXIT);
}



/*
	gray_test()
	Runs the gray engine.
*/

void gray_test(void)
{
	extern Image res_illustration_start;
	Image *illustration = &res_illustration_start;

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

//			gimage(illustration, 0, 0);
//			gclear_area(64, 0, 127, 63);

			gtext("light", 78, 6);
			print_int(delays[0], 103, 6);

			gtext("dark", 78, 15);
			print_int(delays[1], 103, 15);

			gtext(">", 70, selected ? 15 : 6);
			gupdate();
		}

		changed = 0;

		key = getkey_opt(Getkey_RepeatArrowKeys, 1);
		if(key == KEY_EXIT) break;

		changed = 1;

		switch(key)
		{
		case KEY_F1:
			delays[0] = 860;
			delays[1] = 1298;
			break;
		case KEY_F2:
			delays[0] = 912;
			delays[1] = 1343;
			break;
		case KEY_F3:
			delays[0] = 993;
			delays[1] = 1609;
			break;
		case KEY_F6:
			selected = !selected;
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
	main_menu()
	Displays the main menu and returns user's choice.

	@return	User choice. 0 means EXIT, other numbers are applications.
*/
int main_menu(void)
{
	extern unsigned int bgint, egint;
	const char *mpu_names[] = {
		"MPU_Unkown",
		"MPU_SH7337",
		"MPU_SH7355",
		"MPU_SH7305",
		"MPU_SH7724",
		"MPU Error !"
	};
	int key;

	dclear();

	print("gint test application", 19, 1);
	dline(19, 8, 107, 8, Color_Black);

	print("[1] Keyboard", 10, 17);
	print("[2] Bitmap drawing", 10, 24);
	print("[3] Text rendering", 10, 31);
	print("[4] Gray engine", 10, 38);

	print("mpu type:", 2, 49);
	print(mpu_names[MPU_CURRENT < 5 ? MPU_CURRENT : 5], 50, 48);
	print("gint size:", 2, 56);
	print_hex(&egint - &bgint, 50, 56);

	dupdate();

	while(1)
	{
		key = getkey();
		if(key == KEY_EXIT) return 0;

		if(key == KEY_1) return 1;
		if(key == KEY_2) return 2;
		if(key == KEY_3) return 3;
		if(key == KEY_4) return 4;
		if(key == KEY_5) return 5;
	}

	return 0;
}

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

#include <../src/timer/timer_internals.h>
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

/*
	main()
	Handles application calls.

	@return	0.
*/
int main(void)
{
	extern Font res_font_start;
	Font *font = &res_font_start;

	text_configure(font);

	int app;

	while(1)
	{
		app = main_menu();
		if(!app) break;

		if(app == 1) keyboard_test();
		if(app == 2) bitmap_test();
		if(app == 3) text_test();
		if(app == 4) gray_test();
		if(app == 5) debug();
	}

	return 0;
}
