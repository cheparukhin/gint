#include "gintdemo.h"
#include <gray.h>
#include <keyboard.h>
#include <mpu.h>

/*
	test_gray()
	Lets the user set the gray delays and see the results.
*/

static void draw(int delay1, int delay2, int selected)
{
	extern image_t res_opt_gray;
	extern font_t res_font_modern;
	uint32_t *vl = gray_lightVRAM();
	uint32_t *vd = gray_darkVRAM();

	gclear();
	locate(1, 1, "Gray engine");

	for(int i = 0; i < 64; i++)
	{
		int offset = (i << 2) + 3;
		vl[offset] = -(!(i & 16));
		vd[offset] = -(i < 32);
	}

	text_configure(&res_font_modern, color_black);
	gtext(13, 17, "light");
	gtext(13, 31, "dark");

	text_configure(NULL, color_black);
	print(4, 4, "%d", delay1);
	print(4, 6, "%d", delay2);

	locate(3, selected ? 6 : 4, "\x02");

	gimage(0, 56, &res_opt_gray);
	gupdate();
}

void test_gray(void)
{
	int delays[2]; // { light, dark }
	int key, changed = 1;
	int selected = 0;

	gray_getDelays(delays, delays + 1);
	gray_start();

	while(1)
	{
		if(changed)
		{
			gray_setDelays(delays[0], delays[1]);
			draw(delays[0], delays[1], selected);
		}
		changed = 0;

		key = getkey_opt(getkey_repeat_arrow_keys, 25);
		if(key == KEY_EXIT) break;

		changed = 1;

		switch(key)
		{
		case KEY_F1:
			selected = !selected;
			break;
		case KEY_F2:
			delays[0] = 912;
			delays[1] = 1343;
			break;
		case KEY_F3:
			delays[0] = 993;
			delays[1] = 1609;
			break;
		case KEY_F4:
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
